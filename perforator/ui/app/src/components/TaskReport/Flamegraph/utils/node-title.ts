import type { FormatNode } from 'src/models/Profile';


export type ReadString = (id?: number) => string;

export type StringModifier = (s: string) => string;

export function getNodeTitleFull(readString: ReadString, maybeShorten: StringModifier, node: FormatNode): string {
    const kind = readString(node.kind);
    let nodeTitle = maybeShorten(readString(node.textId)) + ' ' + readString(node.file);
    if (kind !== '') {
        nodeTitle += ` (${kind})`;
    }
    if (node.inlined) {
        nodeTitle += ' (inlined)';
    }
    return nodeTitle;
}
