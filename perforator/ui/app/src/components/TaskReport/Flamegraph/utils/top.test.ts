import { describe, expect, it } from '@jest/globals';

import type { ProfileData } from 'src/models/Profile';

import { calculateTopForTable } from './top';


const rows: ProfileData['rows'] = [
    [
        { parentIndex: -1, textId: 0, eventCount: 100, sampleCount: 100 },
    ],
    [
        { parentIndex: 0, textId: 1, eventCount: 50, sampleCount: 50 },
        { parentIndex: 0, textId: 2, eventCount: 50, sampleCount: 50 },
    ],
    [
        { parentIndex: 0, textId: 3, eventCount: 50, sampleCount: 50 },
        { parentIndex: 1, textId: 2, eventCount: 49, sampleCount: 49 },
    ],
];

const profileData = { rows, stringTable: ['all', 'a', 'b', 'c', 'cycles', 'function'], meta: { version: 1, eventType: 4, frameType: 5 } };

describe('top', () => {
    const topData = calculateTopForTable(profileData.rows, profileData.stringTable.length).sort((a, b) => b.textId - a.textId);
    it('should work for example data', () => {
        expect(topData).toMatchSnapshot();
    });
    it('there should be no function with more total events than root', () => {
        const topEventCount = profileData.rows[0][0].eventCount;

        for (const topEntry of topData) {
            expect(topEntry['all.eventCount']).toBeLessThanOrEqual(topEventCount);
        }
    });
    it('there should be no self time on root', () => {
        const rootTopData = topData.find((topEntry) => topEntry.textId === 0)!;
        expect(rootTopData['self.eventCount']).toBe(0);
    });
    it('recursive functions should be correctly accounted in total time', () => {
        const topEntry = topData.find((entry) => entry.textId === 2)!;
        expect(topEntry['all.eventCount']).toBe(50);
    });
});
