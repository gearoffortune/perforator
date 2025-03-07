import React from 'react';

import { Button, CopyToClipboard, Popup, type PopupProps } from '@gravity-ui/uikit';

import { Hotkey } from 'src/components/Hotkey/Hotkey';
import { uiFactory } from 'src/factory';
import type { StringifiedNode } from 'src/models/Profile';

import type { GetStateFromQuery, SetStateFromQuery } from './query-utils';
import { parseStacks, stringifyStacks } from './query-utils';
import type { QueryKeys } from './renderer';


export type PopupData = { offset: [number, number]; node: StringifiedNode; coords: [number, number] };

function getHref(node: StringifiedNode) {
    return uiFactory().goToDefinitionHref(node);
}

type ContextMenuProps = {
    popupData: PopupData;
    anchorRef: PopupProps['anchorRef'];
    onClosePopup: () => void;
    setQuery: SetStateFromQuery<QueryKeys>;
    getQuery: GetStateFromQuery<QueryKeys>;
};
export const ContextMenu: React.FC<ContextMenuProps> = ({ popupData, anchorRef, onClosePopup, setQuery, getQuery }) => {
    const href = getHref(popupData.node);
    const hasFile = Boolean(popupData.node.file);
    const shouldShowGoTo = (
        hasFile &&
        popupData.node.frameOrigin !== 'kernel' &&
        href
    );
    const commonButtonProps = {
        view: 'flat',
        width: 'max',
        onClick: onClosePopup,
        pin: 'brick-brick',
        size: 'l',
    } as const;

    return <Popup
        altBoundary={true}
        open={Boolean(popupData)}
        anchorRef={anchorRef}
        offset={popupData.offset}
        contentClassName={'flamegraph__popup'}
        placement={['top-start']}
        onEscapeKeyDown={onClosePopup}
    >
        <CopyToClipboard text={popupData.node.textId} >
            {() => <Button
                {...commonButtonProps}
            >

                Copy name
            </Button>}
        </CopyToClipboard>
        {/* eslint-disable-next-line no-nested-ternary */}
        {shouldShowGoTo ? (
            <Button
                {...commonButtonProps}
                href={href}
                target="_blank"
            >
                Go to source
            </Button>

        ) : hasFile ? (
            <CopyToClipboard text={popupData.node.file} >
                {() => <Button
                    {...commonButtonProps}
                >
                    Copy file path
                </Button>}
            </CopyToClipboard>
        ) : null}
        <Button
            {...commonButtonProps}
            onClick={() => {
                const omitted = parseStacks(getQuery('omittedIndexes', '') || '');
                omitted.push(popupData.coords);
                setQuery({ omittedIndexes: stringifyStacks(omitted) });
                onClosePopup();
            }}
        >
            Omit stack
            <Hotkey value="alt+click" />
        </Button>
    </Popup>;
};
