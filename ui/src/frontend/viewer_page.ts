
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

import {executeQuery} from '../common/actions';
import {QueryResponse} from '../common/queries';
import {EngineConfig} from '../common/state';

import {globals} from './globals';
import {OverviewTimeline} from './overview_timeline';
import {createPage} from './pages';
import {PanAndZoomHandler} from './pan_and_zoom_handler';
import {ScrollingTrackDisplay} from './scrolling_track_display';
import {TimeAxis} from './time_axis';
import {TimeScale} from './time_scale';
import {TRACK_SHELL_WIDTH} from './track_component';


const QueryTable: m.Component<{}, {}> = {
  view() {
    const resp = globals.queryResults.get('command') as QueryResponse;
    if (resp === undefined) {
      return m('');
    }
    const cols = [];
    for (const col of resp.columns) {
      cols.push(m('td', col));
    }
    const header = m('tr', cols);

    const rows = [];
    for (let i = 0; i < resp.rows.length; i++) {
      const cells = [];
      for (const col of resp.columns) {
        cells.push(m('td', resp.rows[i][col]));
      }
      rows.push(m('tr', cells));
    }
    return m(
        'div',
        m('header.overview',
          `Query result - ${resp.durationMs} ms`,
          m('span.code', resp.query)),
        resp.error ?
            m('.query-error', `SQL error: ${resp.error}`) :
            m('table.query-table', m('thead', header), m('tbody', rows)));
  },
};

export const OVERVIEW_QUERY_ID = 'overview_query';

/**
 * Top-most level component for the viewer page. Holds tracks, brush timeline,
 * panels, and everything else that's part of the main trace viewer page.
 */
const TraceViewer = {
  oninit() {
    this.width = 0;
    this.visibleWindowMs = {start: 1000000, end: 2000000};
    this.maxVisibleWindowMs = {start: 0, end: 10000000};
    this.timeScale = new TimeScale(
        [this.visibleWindowMs.start, this.visibleWindowMs.end],
        [0, this.width - TRACK_SHELL_WIDTH]);
  },
  oncreate(vnode) {
    this.onResize = () => {
      const rect = vnode.dom.getBoundingClientRect();
      this.width = rect.width;
      this.timeScale.setLimitsPx(0, this.width - TRACK_SHELL_WIDTH);
      m.redraw();
    };

    // Have to redraw after initialization to provide dimensions to view().
    setTimeout(() => this.onResize());

    // Once ResizeObservers are out, we can stop accessing the window here.
    window.addEventListener('resize', this.onResize);

    const panZoomEl =
        vnode.dom.getElementsByClassName('tracks-content')[0] as HTMLElement;

    // TODO: ContentOffsetX should be defined somewhere central.
    // Currently it lives here, in canvas wrapper, and in track shell.
    this.zoomContent = new PanAndZoomHandler({
      element: panZoomEl,
      contentOffsetX: TRACK_SHELL_WIDTH,
      onPanned: (pannedPx: number) => {
        const deltaMs = this.timeScale.deltaPxToDurationMs(pannedPx);
        this.visibleWindowMs.start += deltaMs;
        this.visibleWindowMs.end += deltaMs;
        this.timeScale.setLimitsMs(
            this.visibleWindowMs.start, this.visibleWindowMs.end);
        m.redraw();
      },
      onZoomed: (zoomedPositionPx: number, zoomPercentage: number) => {
        const totalTimespanMs =
            this.visibleWindowMs.end - this.visibleWindowMs.start;
        const newTotalTimespanMs = totalTimespanMs * zoomPercentage;

        const zoomedPositionMs =
            this.timeScale.pxToMs(zoomedPositionPx) as number;
        const positionPercentage =
            (zoomedPositionMs - this.visibleWindowMs.start) / totalTimespanMs;

        this.visibleWindowMs.start =
            zoomedPositionMs - newTotalTimespanMs * positionPercentage;
        this.visibleWindowMs.end =
            zoomedPositionMs + newTotalTimespanMs * (1 - positionPercentage);
        this.timeScale.setLimitsMs(
            this.visibleWindowMs.start, this.visibleWindowMs.end);
        m.redraw();
      }
    });
  },
  onremove() {
    window.removeEventListener('resize', this.onResize);
    this.zoomContent.shutdown();
  },
  view() {
    const onBrushedMs = (start: number, end: number) => {
      this.visibleWindowMs.start = start;
      this.visibleWindowMs.end = end;
      this.timeScale.setLimitsMs(
          this.visibleWindowMs.start, this.visibleWindowMs.end);
      m.redraw();
    };

    const engine: EngineConfig = globals.state.engines['0'];
    if (engine) {
      const sourceEqual =
          (!this.engineSource || typeof this.engineSource === 'string') ?
          engine.source === this.engineSource :
          typeof engine.source !== 'string' &&
              engine.source.name === this.engineSource.name;

      if (engine && engine.ready && !sourceEqual) {
        this.engineSource = engine.source;
        console.log('File loaded. Executing query..');
        globals.dispatch(executeQuery(
            engine.id,
            OVERVIEW_QUERY_ID,
            'select round(ts/1e5) as rts, sum(dur)/1e5 as load, upid, ' +
                'thread.name from slices inner join thread using(utid) where ' +
                'depth = 0 group by rts, upid limit 100'));
      }
    }
    const resp = globals.queryResults.get(OVERVIEW_QUERY_ID) as QueryResponse;
    if (resp !== this.overviewQueryResponse) {
      console.log('Query Executed.');
      this.overviewQueryResponse = resp;

      const times = resp.rows.map(processLoad => processLoad.rts as number);
      const minTime = Math.min(...times);
      const duration = (Math.max(...times) - minTime) * 1000;

      const previousDuration =
          this.maxVisibleWindowMs.end - this.maxVisibleWindowMs.start;
      const startPercent =
          (this.visibleWindowMs.start - this.maxVisibleWindowMs.start) /
          previousDuration;
      const endPercent =
          (this.visibleWindowMs.end - this.maxVisibleWindowMs.start) /
          previousDuration;

      this.maxVisibleWindowMs.start = 0;
      this.maxVisibleWindowMs.end = duration;

      this.visibleWindowMs.start = duration * startPercent;
      this.visibleWindowMs.end = duration * endPercent;

      // TODO: Add tracks.
    }

    return m(
        '.page',
        {
          style: {
            width: '100%',
            height: '100%',
          },
        },
        m('header.overview', 'Big picture'),
        m(OverviewTimeline, {
          visibleWindowMs: this.visibleWindowMs,
          maxVisibleWindowMs: this.maxVisibleWindowMs,
          onBrushedMs
        }),
        m(QueryTable),
        m('.tracks-content',
          {
            style: {
              width: '100%',
              height: 'calc(100% - 145px)',
            }
          },
          m('header.tracks-content', 'Tracks'),
          m(TimeAxis, {
            timeScale: this.timeScale,
            contentOffset: TRACK_SHELL_WIDTH,
            visibleWindowMs: this.visibleWindowMs,
          }),
          m(ScrollingTrackDisplay, {
            timeScale: this.timeScale,
            visibleWindowMs: this.visibleWindowMs,
          })));
  },
} as m.Component<{}, {
  visibleWindowMs: {start: number, end: number},
  maxVisibleWindowMs: {start: number, end: number},
  onResize: () => void,
  timeScale: TimeScale,
  width: number,
  zoomContent: PanAndZoomHandler,
  engineSource: string | File,
  overviewQueryResponse: QueryResponse,
}>;

export const ViewerPage = createPage({
  view() {
    return m(TraceViewer);
  }
});
