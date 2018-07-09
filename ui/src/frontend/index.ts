/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import * as m from 'mithril';
import {Track} from './track';
import {CanvasController} from './canvas_controller';
import {ScrollableContainer} from './scrollable_container';
import {CanvasWrapper} from './canvas_wrapper';
import {TrackCanvasContext} from './track_canvas_context';

export const Frontend = {
  oninit() {

    this.width = 1000;
    this.height = 400;

    this.cc = new CanvasController(this.width, this.height);
  },
  view({}) {
    const canvasScrollOffset = this.cc.getCanvasScrollOffset();
    const cctx = this.cc.getContext();

    this.cc.clear();

    return m('.frontend',
      {
        style: {
          position: 'relative',
          width: this.width.toString() + 'px'
        }
      },
      m(ScrollableContainer,
        {
          width: this.width,
          height: this.height,
          contentHeight: 1000,
          onPassiveScroll: (scrollTop: number) => {
            this.cc.updateScrollOffset(scrollTop);
            this.cc.getContext().setYOffset(scrollTop * -1);
            m.redraw();
          },
        },
        m(CanvasWrapper, {
          scrollOffset: canvasScrollOffset,
          canvasElement: this.cc.getCanvasElement()
        }),
        m(Track, { name: 'Track 1', cctx: new TrackCanvasContext(
          cctx, {top: 0, left: 0, width: this.width, height: 90}
        ), top: 0 }),
        m(Track, { name: 'Track 2', cctx: new TrackCanvasContext(cctx,
          {top: 100, left: 0, width: this.width, height: 90}
        ), top: 100 }),
        m(Track, { name: 'Track 3', cctx: new TrackCanvasContext(
          cctx, {top: 200, left: 0, width: this.width, height: 90}
        ), top: 200 }),
        m(Track, { name: 'Track 4', cctx: new TrackCanvasContext(
          cctx, {top: 300, left: 0, width: this.width, height: 90}
        ), top: 300 }),
        m(Track, { name: 'Track 5', cctx: new TrackCanvasContext(
          cctx, {top: 400, left: 0, width: this.width, height: 90}
        ), top: 400 }),
        m(Track, { name: 'Track 6', cctx: new TrackCanvasContext(
          cctx, {top: 500, left: 0, width: this.width, height: 90}
        ), top: 500 }),
        m(Track, { name: 'Track 7', cctx: new TrackCanvasContext(
          cctx, {top: 600, left: 0, width: this.width, height: 90}
        ), top: 600 }),
        m(Track, { name: 'Track 8', cctx: new TrackCanvasContext(
          cctx, {top: 700, left: 0, width: this.width, height: 90}
        ), top: 700 }),
        m(Track, { name: 'Track 9', cctx: new TrackCanvasContext(
          cctx, {top: 800, left: 0, width: this.width, height: 90}
        ), top: 800 }),
        m(Track, { name: 'Track 10', cctx: new TrackCanvasContext(
          cctx, {top: 900, left: 0, width: this.width, height: 90}
        ), top: 900 }),
      ),
    );
  },
} as m.Component<{width: number, height: number}, {
  oninit: () => void,
  cc: CanvasController,
  width: number,
  height: number
}>;
