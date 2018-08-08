// Copyright (C) 2018 The Android Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

import * as m from 'mithril';

import {assertExists} from '../base/logging';

import {globals} from './globals';
import {lightRedrawer} from './light_redrawer';
import {Panel} from './panel';
import {TrackPanel} from './track_panel';

/**
 * The backing canvas height is CANVAS_OVERDRAW_FACTOR * visible height.
 */
const CANVAS_OVERDRAW_FACTOR = 2;

function getCanvasOverdrawHeightPerSide(visibleHeight: number) {
  const overdrawHeight = (CANVAS_OVERDRAW_FACTOR - 1) * visibleHeight;
  return overdrawHeight / 2;
}

function getCanvasYStart(visibleHeight: number, containerScrollTop: number) {
  return containerScrollTop - getCanvasOverdrawHeightPerSide(visibleHeight);
}

function setCanvasDimensions(
    ctx: CanvasRenderingContext2D,
    containerWidth: number,
    containerHeight: number) {
  const dpr = window.devicePixelRatio;
  ctx.canvas.width = containerWidth * dpr;
  ctx.canvas.height = containerHeight * CANVAS_OVERDRAW_FACTOR * dpr;
  ctx.scale(dpr, dpr);
}

type CanvasScrollingContainerVnode =
    m.VnodeDOM<{}, ScrollingPanelContainerState>;

function updateDimensionsFromDom(vnode: CanvasScrollingContainerVnode) {
  const rect = vnode.dom.getBoundingClientRect();
  vnode.state.domWidth = rect.width;
  vnode.state.domHeight = rect.height;
  setCanvasDimensions(
      assertExists(vnode.state.ctx),
      vnode.state.domWidth,
      vnode.state.domHeight);
  m.redraw();
}

/**
 * Stores a panel, and associated metadata.
 */
interface PanelStruct {
  height: number;
  panel: Panel;
  key: string;
}

const PanelComponent = {
  view({attrs}) {
    return m('.panel', {
      style: {
        height: `${attrs.panelStruct.height}px`,
        width: '100%',
        overflow: 'hidden',
        position: 'absolute',
        top: `${attrs.yStart}px`,
      },
      key: attrs.panelStruct.key,
    });
  },

  onupdate({dom, attrs}) {
    attrs.panelStruct.panel.updateDom(dom);
  }
} as m.Component<{panelStruct: PanelStruct, yStart: number}>;

function panelIsOnCanvas(
    panelYBoundsOnCanvas: {start: number, end: number}, canvasHeight: number) {
  return panelYBoundsOnCanvas.end > 0 &&
      panelYBoundsOnCanvas.start < canvasHeight;
}

function renderPanelCanvas(
    ctx: CanvasRenderingContext2D,
    width: number,
    yStartOnCanvas: number,
    panelStruct: PanelStruct) {
  ctx.save();
  ctx.translate(0, yStartOnCanvas);
  const clipRect = new Path2D();
  clipRect.rect(0, 0, width, panelStruct.height);
  ctx.clip(clipRect);

  panelStruct.panel.renderCanvas(ctx);

  ctx.restore();
}

function drawCanvas(state: ScrollingPanelContainerState) {
  if (!state.ctx) return;
  const canvasHeight = state.domHeight * CANVAS_OVERDRAW_FACTOR;
  state.ctx.clearRect(0, 0, state.domWidth, canvasHeight);
  const canvasYStart =
      state.scrollTop - getCanvasOverdrawHeightPerSide(state.domHeight);

  let panelYStart = 0;
  for (const panelStruct of state.keyToPanelStructs.values()) {
    const yStartOnCanvas = panelYStart - canvasYStart;
    const panelYBoundsOnCanvas = {
      start: yStartOnCanvas,
      end: yStartOnCanvas + panelStruct.height
    };
    if (!panelIsOnCanvas(panelYBoundsOnCanvas, canvasHeight)) {
      panelYStart += panelStruct.height;
      continue;
    }

    renderPanelCanvas(state.ctx, state.domWidth, yStartOnCanvas, panelStruct);
    panelYStart += panelStruct.height;
  }
}

interface ScrollingPanelContainerState {
  domWidth: number;
  domHeight: number;
  scrollTop: number;
  ctx: CanvasRenderingContext2D|null;
  keyToPanelStructs: Map<string, PanelStruct>;
  panelDisplayOrder: string[];

  // We store this function so we can remove it.
  onResize: () => void;
  canvasRedrawer: () => void;
}

export const ScrollingPanelContainer = {
  oninit({state}) {
    // These values are updated with proper values in oncreate.
    this.domWidth = 0;
    this.domHeight = 0;
    this.scrollTop = 0;
    this.ctx = null;
    this.keyToPanelStructs = new Map<string, PanelStruct>();
    this.canvasRedrawer = () => drawCanvas(state);
    lightRedrawer.addCallback(this.canvasRedrawer);
  },

  oncreate(vnode) {
    // Save the canvas context in the state.
    const canvas = vnode.dom.querySelector('.main-canvas') as HTMLCanvasElement;
    const ctx = canvas.getContext('2d');
    if (!ctx) {
      throw Error('Cannot create canvas context');
    }
    this.ctx = ctx;

    // updateDimensionsFromDom calls m.redraw, which cannot be called while a
    // redraw is already happening. Use setTimeout to do it at the end of the
    // current redraw.
    setTimeout(() => updateDimensionsFromDom(vnode));

    // Save the resize handler in the state so we can remove it later.
    // TODO: Encapsulate resize handling better.
    this.onResize = () => updateDimensionsFromDom(vnode);

    // Once ResizeObservers are out, we can stop accessing the window here.
    window.addEventListener('resize', this.onResize);

    vnode.dom.addEventListener('scroll', () => {
      vnode.state.scrollTop = vnode.dom.scrollTop;
      m.redraw();
    }, {passive: true});
  },

  onremove() {
    window.removeEventListener('resize', this.onResize);
    lightRedrawer.removeCallback(this.canvasRedrawer);
  },

  view() {
    // TODO: Handle panel deletion.
    // Create all the track panels if they don't already exist.
    for (const id of globals.state.displayedTrackIds) {
      const trackState = globals.state.tracks[id];
      // Makeshift name mangling.
      let panelStruct = this.keyToPanelStructs.get('track-' + id);
      if (panelStruct === undefined) {
        panelStruct = {
          panel: new TrackPanel(trackState),
          height: trackState.height,
          key: id,
        };
        this.keyToPanelStructs.set('track-' + id, panelStruct);
      }
    }

    // Ordered list of panel to display, by key. Store this in state so canvas
    // can use it.
    this.panelDisplayOrder =
        globals.state.displayedTrackIds.map(id => 'track-' + id);
    const panelComponents: m.Children[] = [];

    let yStart = 0;
    for (const key of this.panelDisplayOrder) {
      const panelStruct = assertExists(this.keyToPanelStructs.get(key));
      panelComponents.push(m(PanelComponent, {panelStruct, yStart}));
      yStart += panelStruct.height;
    }

    let totalContentHeight = 0;
    for (const panelStruct of this.keyToPanelStructs.values()) {
      totalContentHeight += panelStruct.height;
    }
    const canvasYStart = getCanvasYStart(this.domHeight, this.scrollTop);

    return m(
        '.scrolling-panel-container',
        // Since the canvas is overdrawn and continuously repositioned, we need
        // the canvas to be in a div with overflow hidden and height equaling
        // the height of the content to prevent scrolling height from growing.
        m('.scroll-limiter',
          {
            style: {
              height: `${totalContentHeight}px`,
              overflow: 'hidden',
              position: 'relative',
            }
          },
          m('canvas.main-canvas', {
            style: {
              height: `${this.domHeight * CANVAS_OVERDRAW_FACTOR}px`,
              // translateY is allegedly better than updating 'top' because it
              // doesn't trigger layout.
              transform: `translateY(${canvasYStart}px)`,
              width: '100%',
              position: 'absolute',
              'background-color': '#eee',
            }
          }),
          ...panelComponents));
  },
} as m.Component<{}, ScrollingPanelContainerState>;
