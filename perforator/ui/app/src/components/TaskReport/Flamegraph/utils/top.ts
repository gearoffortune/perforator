import type { FormatNode, ProfileData, StringifiableFields } from 'src/models/Profile';

import type { H, I } from '../renderer';


export type TableFunctionTop = FunctionTop

export function calculateTopForTable(data: ProfileData['rows'], stringTableLength: number) {
    const topData = calculateTop(data, stringTableLength);

    const res: (TableFunctionTop)[] = [];
    for (const value of topData.values()) {
        delete value.shortestPath;
        res.push(value);
    }

    return res;
}


function populateWithSelfEventCount(rows: ProfileData['rows']) {
    for (let h = 0; h < rows.length; h++) {
        const row = rows[h];
        for (let i = 0; i < rows[h].length; i++) {
            row[i].selfEventCount = row[i].eventCount;
            row[i].selfSampleCount = row[i].sampleCount;
            if (row[i].parentIndex !== -1) {
                const parentIndex = row[i].parentIndex;
                rows[h - 1][parentIndex].selfEventCount! -= row[i].eventCount;
                rows[h - 1][parentIndex].selfSampleCount! -= row[i].sampleCount;
            }
        }
    }
}

function populateWithChildren(rows: ProfileData['rows']) {
    for (let h = rows.length - 1; h > 0; h--) {
        const row = rows[h];
        for (let i = 0; i < row.length; i++) {
            const parentNode = rows[h - 1][row[i].parentIndex];
            if (!parentNode.childrenIndices) {
                parentNode.childrenIndices = new Set();
            }
            parentNode.childrenIndices.add(i);
        }
    }
}

type Field = 'eventCount' | 'sampleCount';

type CountType = 'self' | 'all'

type TopKeys = `${CountType}.${Field}`;

type FunctionTop = Record<TopKeys, number> & Pick<FormatNode, StringifiableFields | 'inlined'> &
{ shortestPath?: I[] }

function getNodeKeyFull(len: number, n: FormatNode) {
    return len ** 2 * (n.kind ?? 0) + (n.file ?? 0) * len + n.textId + (n.inlined ? 1 : 0);
}

export function calculateTop(rows: ProfileData['rows'], stringTableLength: number) {

    const res: Map<number, FunctionTop> = new Map();


    populateWithSelfEventCount(rows);


    const getNodeKey = getNodeKeyFull.bind(null, stringTableLength);
    for (let h = 0; h < rows.length; h++) {
        const row = rows[h];
        for (let i = 0; i < row.length; i++) {
            const node = row[i];
            const funcKey = getNodeKey(node);
            if (!res.has(funcKey)) {
                res.set(funcKey, {
                    'all.eventCount': 0,
                    'all.sampleCount': 0,
                    'self.eventCount': 0,
                    'self.sampleCount': 0,
                    textId: node.textId,
                    file: node.file,
                    frameOrigin: node.frameOrigin,
                    inlined: node.inlined,
                    kind: node.kind,
                });
            }
            const funcTopData = res.get(funcKey)!;
            funcTopData['self.eventCount'] += node.selfEventCount!;
            funcTopData['self.sampleCount'] += node.selfSampleCount!;
        }
    }

    populateWithChildren(rows);

    calcTotalTime(res, rows, getNodeKey);

    return res;
}

function isSubpath(path: I[], subpath: I[]) {
    if (subpath.length >= path.length) {
        return false;
    }
    for (let i = 0; i < subpath.length; i++) {
        if (path[i] !== subpath[i]) {
            return false;
        }
    }
    return true;
}

// we can't just sum all the `all.eventCount` over all nodes with the same name (because recursion)
// so instead we do DFS and keep the shortest path for every function
function calcTotalTime<K>(res: Map<K, FunctionTop>, rows: FormatNode[][], getNodeTitle: (node: FormatNode) => K) {
    function walker(h: H, i: I, indexesPath: I[]) {
        const node = rows[h][i];
        const key = getNodeTitle(node);
        const funcTopData = res.get(key)!;


        if (!funcTopData.shortestPath || !isSubpath(indexesPath, funcTopData.shortestPath)) {
            funcTopData['all.eventCount'] += node.eventCount;
            funcTopData['all.sampleCount'] += node.sampleCount;
            funcTopData.shortestPath = [...indexesPath];
        }

        for (const childIndex of node.childrenIndices ?? []) {
            walker(h + 1, childIndex, indexesPath.concat(childIndex));
        }
    }

    walker(0, 0, [0]);
}
