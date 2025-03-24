import type { ProfileData } from 'src/models/Profile';

import type { Coordinate } from '../renderer';

import type { ReadString, StringModifier } from './node-title';
import { getNodeTitleFull } from './node-title';


export function search(readString: ReadString, maybeShorten: StringModifier, rows: ProfileData['rows'], query: RegExp): Coordinate[] {
    const res: Coordinate[] = [];

    const getNodeTitle = getNodeTitleFull.bind(null, readString, maybeShorten);

    for (let h = 0; h < rows.length; h++) {
        for (let i = 0; i < rows[h].length; i++) {
            const node = rows[h][i];
            const name = getNodeTitle(node);
            const matched = query.test(name);
            if (matched) {
                res.push([h, i]);
            }
        }
    }

    return res;
}
