import React, { useMemo } from 'react';

import { useNavigate } from 'react-router-dom';

import type { FlamegraphProps, QueryKeys } from '@perforator/flamegraph';
import { calculateTopForTable, Flamegraph, prerenderColors, TopTable } from '@perforator/flamegraph';

import { Loader, Tabs, useThemeType } from '@gravity-ui/uikit';

import { Link } from '../Link/Link';
import { createSuccessToast } from '../../utils/toaster';

import { useTypedQuery } from '../../query-utils';

import './Visualisation.css';

export type Tab = 'flame' | 'top'
export interface VisualisationProps
    extends Pick<FlamegraphProps, 'profileData' | 'theme'> {
    loading: boolean;
}

export const Visualisation: React.FC<VisualisationProps> = ({ profileData, ...props }) => {
    const navigate = useNavigate();
    const [getQuery, setQuery] = useTypedQuery<'tab' | QueryKeys>();
    const tab: Tab = getQuery('tab', 'flame') as Tab;
    const isTopTab = tab === 'top';
    const [isFirstTopRender, setIsFirstTopRender] = React.useState(isTopTab);
    React.useEffect(() => {
        setIsFirstTopRender(isFirstTopRender || isTopTab);
    }, [isFirstTopRender, isTopTab]);
    const theme = useThemeType();

    React.useLayoutEffect(() => {
        if (profileData) {prerenderColors(profileData, { theme });}
    }, [profileData, theme]);

    const isDiff = useMemo(() => Boolean(profileData?.rows?.[0][0].baseEventCount), [profileData])

    const topData = React.useMemo(() => {
        return profileData && isFirstTopRender
            ? calculateTopForTable(profileData.rows, profileData.stringTable.length)
            : null;
    }, [profileData, isFirstTopRender]);

    let content: React.JSX.Element | undefined;

    if (props.loading) {
        content = <Loader />;
    } else {
        if (tab === 'flame') {
            content = (
                <Flamegraph
                goToDefinitionHref={() => ''}    
                profileData={profileData}
                    getState={getQuery}
                    isDiff={isDiff}
                    setState={setQuery}
                    onSuccess={createSuccessToast}
                    userSettings={{monospace: 'default', numTemplating: 'exponent', 'reverseFlameByDefault': true, shortenFrameTexts: 'false', theme: 'system'}}
                    {...props}
                />
            );
        }
        if (tab === 'top' && topData && profileData) {
            content = (
                <TopTable
                    goToDefinitionHref={() => ''}
                    topData={topData}
                    profileData={profileData}
                    userSettings={{monospace: 'default', numTemplating: 'exponent', 'reverseFlameByDefault': true, shortenFrameTexts: 'false', theme: 'system'}}
                    navigate={navigate}
                    getState={getQuery}
                    setState={setQuery}
                    {...props}
                />
            );
        }
    }

    return (
        <div className='visualisation'>
            <Tabs
                className={'visualisation_tabs'}
                activeTab={tab}
                wrapTo={(item, node) => (
                    <Link key={item.id} href={`?tab=${item?.id}`}>
                        {node}
                    </Link>
                )}
                items={[
                    { id: 'flame', title: 'Flamegraph' },
                    { id: 'top', title: 'Top' },
                ]}
                onSelectTab={() => {}}
            />
            {content}
        </div>
    );
};
