import React from 'react';

import { CopyCheck } from '@gravity-ui/icons';
import type { MenuItemProps, PopupProps } from '@gravity-ui/uikit';
import { CopyToClipboard, Icon, Menu, Popup } from '@gravity-ui/uikit';

import { Hotkey } from 'src/components/Hotkey/Hotkey';
import { uiFactory } from 'src/factory';
import type { StringifiedNode } from 'src/models/Profile';
import { createSuccessToast } from 'src/utils/toaster';

import { getAtLessPath } from './file-path';
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
    const commonButtonProps: Partial<MenuItemProps> = {
        onClick: onClosePopup,
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
        <Menu>
            <CopyToClipboard text={popupData.node.textId} >
                {() => <Menu.Item
                    {...commonButtonProps}
                    onClick={() => {
                        createSuccessToast({ renderIcon: () => <Icon data={CopyCheck}/>, name: 'copy', content: 'Name copied to clipboard' });
                        onClosePopup();
                    }}
                >

                Copy name
                </Menu.Item>}
            </CopyToClipboard>
            {shouldShowGoTo ? (
                <Menu.Item
                    {...commonButtonProps}
                    href={href}
                    target="_blank"
                >
                Go to source
                </Menu.Item>

            ) : null}
            {hasFile ? (
                <CopyToClipboard text={getAtLessPath(popupData.node)} >
                    {() => <Menu.Item
                        {...commonButtonProps}
                        onClick={() => {
                            createSuccessToast({ renderIcon: () => <Icon data={CopyCheck}/>, name: 'copy', content: 'File path copied to clipboard' });
                            onClosePopup();
                        }}
                    >
                    Copy file path
                    </Menu.Item>}
                </CopyToClipboard>
            ) : null}
            <Menu.Group>
                <Menu.Item
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
                </Menu.Item>
            </Menu.Group>
        </Menu>
    </Popup>;
};
