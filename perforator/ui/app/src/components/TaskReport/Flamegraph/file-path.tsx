import type { StringifiedNode } from 'src/models/Profile';


export const getAtLessPath = (node: StringifiedNode) => {
    if (!node.file) {
        return '';
    }
    const s = node.file;

    if (node.frameOrigin === 'python' || node.frameOrigin === 'kernel') {
        return s.replace('@', '');
    }

    return s.replace('@/', '');
};
