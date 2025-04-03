import React, { useState } from 'react';

import { Xmark } from '@gravity-ui/icons';
import BarsAscendingAlignLeftArrowUpIcon from '@gravity-ui/icons/svgs/bars-ascending-align-left-arrow-up.svg?raw';
import BarsDescendingAlignLeftArrowDownIcon from '@gravity-ui/icons/svgs/bars-descending-align-left-arrow-down.svg?raw';
import FunnelIcon from '@gravity-ui/icons/svgs/funnel.svg?raw';
import FunnelXmarkIcon from '@gravity-ui/icons/svgs/funnel-xmark.svg?raw';
import MagnifierIcon from '@gravity-ui/icons/svgs/magnifier.svg?raw';
import { Button, Icon, Loader } from '@gravity-ui/uikit';

import { Hotkey } from 'src/components/Hotkey/Hotkey';
import type { ProfileData } from 'src/models/Profile';
import type { UserSettings } from 'src/providers/UserSettingsProvider/UserSettings.ts';

import type { PopupData } from './ContextMenu.tsx';
import { ContextMenu } from './ContextMenu.tsx';
import { useTypedQuery } from './query-utils.ts';
import { RegexpDialog } from './RegexpDialog/RegexpDialog.tsx';
import type { QueryKeys } from './renderer.ts';
import { FlamegraphOffseter, renderFlamegraph as newFlame } from './renderer.ts';
import { readNodeStrings } from './utils/read-string.ts';

import './Flamegraph.scss';


export interface FlamegraphProps {
    isDiff: boolean;
    theme: 'light' | 'dark';
    userSettings: UserSettings;
    profileData: ProfileData | null;
    loading: boolean;
}

export const Flamegraph: React.FC<FlamegraphProps> = ({ isDiff, theme, userSettings, profileData, loading }) => {
    const flamegraphContainer = React.useRef<HTMLDivElement | null>(null);
    const canvasRef = React.useRef<HTMLDivElement | null>(null);
    const [getQuery, setQuery] = useTypedQuery<QueryKeys>();
    const [popupData, setPopupData] = useState<null | PopupData>(null);
    const [showDialog, setShowDialog] = useState(false);
    const flamegraphOffsets = React.useRef<FlamegraphOffseter | null>(null);
    const search = getQuery('flamegraphQuery');
    const reverse = (getQuery('flamegraphReverse') ?? String(userSettings.reverseFlameByDefault)) === 'true';

    React.useEffect(() => {
        if (profileData) {
            const getCssVariable = (variable: string) => {
                return getComputedStyle(flamegraphContainer.current!).getPropertyValue(variable);
            };
            const levelHeight = parseInt(getCssVariable('--flamegraph-level-height')!);
            flamegraphOffsets.current = new FlamegraphOffseter(profileData, { reverse, levelHeight });
        }
    }, [profileData, reverse]);

    const handleSearch = React.useCallback(() => {
        setShowDialog(true);
    }, []);

    const handleReverse = React.useCallback(() => {
        setQuery({ 'flamegraphReverse': String(!reverse) });
    }, [reverse, setQuery]);

    const handleSearchReset = React.useCallback(() => {
        setQuery({ flamegraphQuery: false });
    }, [setQuery]);
    const haveOmittedNodes = Boolean(getQuery('omittedIndexes'));
    const keepOnlyFound = getQuery('keepOnlyFound') === 'true';
    const switchKeepOnlyFound = React.useCallback(() => {
        setQuery({ keepOnlyFound: !keepOnlyFound ? 'true' : false });
    }, [keepOnlyFound, setQuery]);
    const handleOmittedNodesReset = React.useCallback(() => {
        setQuery({ omittedIndexes: false });
    }, [setQuery]);

    const handleSearchUpdate = (text: string, exactMatch?: boolean) => {
        setQuery({ 'flamegraphQuery': encodeURIComponent(text), exactMatch: exactMatch ? 'true' : undefined });
        setShowDialog(false);
    };
    const exactMatch = getQuery('exactMatch');


    React.useEffect(() => {
        if (flamegraphContainer.current && profileData && flamegraphOffsets.current) {
            flamegraphContainer.current.style.setProperty('--flamegraph-font', userSettings.monospace === 'system' ? 'monospace' : 'var(--g-font-family-monospace)');

            const renderOptions = {
                setState: setQuery,
                getState: getQuery,
                theme,
                userSettings,
                isDiff,
                searchPattern: search ? exactMatch === 'true' ? decodeURIComponent(search) : RegExp(decodeURIComponent(search)) : null,
                reverse,
                keepOnlyFound,
            };

            return newFlame(flamegraphContainer.current, profileData, flamegraphOffsets.current, renderOptions);
        }
        return () => {};
    }, [exactMatch, getQuery, isDiff, keepOnlyFound, profileData, reverse, search, setQuery, theme, userSettings]);

    const handleContextMenu = React.useCallback((event: React.MouseEvent) => {
        if (!flamegraphContainer.current || !profileData || !flamegraphOffsets.current) {
            return;
        }
        event.preventDefault();

        const offsetX = event.nativeEvent.offsetX;
        const offsetY = event.nativeEvent.offsetY;
        const coordsClient = flamegraphOffsets.current!.getCoordsByPosition(offsetX, offsetY);

        if (!coordsClient) {
            return;
        }

        const stringifiedNode = readNodeStrings(profileData, coordsClient);
        setPopupData({ offset: [offsetX, -offsetY], node: stringifiedNode, coords: [coordsClient.h, coordsClient.i] });
    }, [profileData]);

    const handleOutsideContextMenu: React.MouseEventHandler = React.useCallback((e) => {
        if (popupData) {
            setPopupData(null);
            e.preventDefault();
            e.stopPropagation();
        }
    }, [popupData]);

    const handleKeyDown = React.useCallback((event: KeyboardEvent)=> {
        if ((event.ctrlKey || event.metaKey) && event.code === 'KeyF') {
            event.preventDefault();
            handleSearch();
        } else if (event.altKey && event.code === 'KeyF') {
            switchKeepOnlyFound();
        } else if (event.key === 'Escape') {
            handleSearchReset();
        }
    }, [handleSearch, handleSearchReset, switchKeepOnlyFound]);

    React.useEffect(() => {
        window.addEventListener('keydown', handleKeyDown);

        return () => {
            window.removeEventListener('keydown', handleKeyDown);
        };
    }, [handleKeyDown]);

    if (loading) {
        return <Loader />;
    }

    const framesCount = profileData?.rows?.reduce((acc, row) => acc + row.length, 0);

    return (
        <>
            <div ref={flamegraphContainer} className="flamegraph">
                <RegexpDialog
                    showDialog={showDialog}
                    onCloseDialog={() => setShowDialog(false)}
                    onSearchUpdate={handleSearchUpdate}
                    initialSearch={search}
                    initialExact={getQuery('exactMatch') === 'true'}
                />
                <div className="flamegraph__header">
                    <h3 className="flamegraph__title">Flame Graph</h3>
                    <div className="flamegraph__buttons">
                        <Button className="flamegraph__button flamegraph__button_reverse" onClick={handleReverse}>
                            <Icon data={reverse ? BarsDescendingAlignLeftArrowDownIcon : BarsAscendingAlignLeftArrowUpIcon}/> Reverse
                        </Button>
                        <Button className="flamegraph__button flamegraph__button_search" onClick={handleSearch}>
                            <Icon className="regexp-dialog__header-icon" data={MagnifierIcon}/>
                        Search
                            <Hotkey value="cmd+F" />
                        </Button>
                        {search ?
                            <Button onClick={switchKeepOnlyFound}>
                                <Icon data={keepOnlyFound ? FunnelXmarkIcon : FunnelIcon}/>
                                {keepOnlyFound ? 'Show all stacks' : 'Show matched stacks'}
                                <Hotkey value="alt+F"/>
                            </Button>
                            : null}
                    </div>
                    <div className="flamegraph__frames-count">Showing {framesCount} frames</div>
                </div>

                <div className="flamegraph__annotations">
                    <div className="flamegraph__status" />
                    <div className="flamegraph__deletion" style={{ display: haveOmittedNodes ? 'inherit' : 'none' }}>
                        <Button
                            className="flamegraph__clear-deletion"
                            view="flat-danger"
                            title="Clear Deletions"
                            onClick={handleOmittedNodesReset}
                        >
                            <Icon data={Xmark} size={20} /> Reset omitted stacks
                        </Button>
                    </div>
                    <div className="flamegraph__match">
                    Matched: <span className="flamegraph__match-value" />
                        <Button
                            className="flamegraph__clear"
                            view="flat-danger"
                            title="Clear"
                            onClick={handleSearchReset}
                        >
                            <Icon data={Xmark} size={20} />
                        </Button>
                    </div>
                </div>

                <div id="profile" className="flamegraph__content">
                    <div ref={canvasRef} onClickCapture={handleOutsideContextMenu} onContextMenu={handleContextMenu}>
                        <canvas className="flamegraph__canvas" />
                    </div>
                    <template className="flamegraph__label-template">
                        <div className="flamegraph__label">
                            <span />
                        </div>
                    </template>
                    <div className="flamegraph__labels-container" />
                    <div className='flamegraph__highlight'>
                        <span />
                    </div>
                </div>
            </div>
            {popupData && (
                <ContextMenu onClosePopup={() => {setPopupData(null);}} popupData={popupData} anchorRef={canvasRef} getQuery={getQuery} setQuery={setQuery} />
            )}
        </>
    );
};
