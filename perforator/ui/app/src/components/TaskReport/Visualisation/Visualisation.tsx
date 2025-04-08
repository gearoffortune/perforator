import React from 'react';

import { Loader, Tabs } from '@gravity-ui/uikit';

import { Link } from 'src/components/Link/Link';

import { Flamegraph, type FlamegraphProps } from '../Flamegraph/Flamegraph';
import { useTypedQuery } from '../Flamegraph/query-utils';
import { calculateTopForTable } from '../Flamegraph/utils/top';
import type { Tab } from '../TaskFlamegraph/TaskFlamegraph';
import { TopTable } from '../Top/TopTable';

import './Visualisation.scss';


export interface VisualisationProps extends FlamegraphProps {
    loading: boolean;
}

export const Visualisation: React.FC<VisualisationProps> = ({ profileData, ...props }) => {
    const [getQuery] = useTypedQuery<'tab'>();
    const tab: Tab = getQuery('tab', 'flame') as Tab;
    const isTopTab = tab === 'top';
    const [isFirstTopRender, setIsFirstTopRender] = React.useState(isTopTab);
    React.useEffect(() => {
        setIsFirstTopRender(isFirstTopRender || isTopTab);
    }, [isFirstTopRender, isTopTab]);

    const topData = React.useMemo(() => {
        return profileData && isFirstTopRender ? calculateTopForTable(profileData.rows, profileData.stringTable.length) : null;
    }, [profileData, isFirstTopRender]);

    let content;

    if (props.loading) {
        content = <Loader />;
    } else {
        if (tab === 'flame' ) {
            content = <Flamegraph profileData={profileData} {...props} />;
        }
        if (tab === 'top' && topData && profileData) {
            content = <TopTable topData={topData} profileData={profileData} />;
        }
    }

    return <React.Fragment>
        <Tabs
            className={'visualisation_tabs'}
            activeTab={tab}
            wrapTo={(item, node) => <Link key={item.id} href={`?tab=${item?.id}`}>{node}</Link>}
            items={[
                { id: 'flame', title: 'Flamegraph' },
                { id: 'top', title: 'Top' },
            ]}
            onSelectTab={() => {}}
        />
        {content}
    </React.Fragment>;
};
