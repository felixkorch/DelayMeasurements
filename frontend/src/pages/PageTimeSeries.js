// Bootstrap
import Card from 'react-bootstrap/Card';

// Libs
import ReactApexChart from "react-apexcharts";
import { DayPicker } from 'react-day-picker';
import 'react-day-picker/dist/style.css';

// React
import { useState, useEffect, useContext, useMemo } from 'react';

// Local
import { Api } from '../Api.js'
import { formatDate, useWindowDimensions } from '../scripts/utilities.js';
import { AppContext } from '../App.js'
import TitleSelect from '../components/TitleSelect.js';

const DARKMODE_FORE_COLOR = '#ffffffc7';
const LIGHTMODE_FORE_COLOR = '#000000';

const MyDayPicker = (props) => {
  const [selected, setSelected] = useState(new Date());
  const ctx = useContext(AppContext);
  const styleColor = ctx.colorMode == 'dark' ? DARKMODE_FORE_COLOR : LIGHTMODE_FORE_COLOR;

  // Calls parent when selected changes
  useEffect(() => {
    props.onSelect(selected);
  }, [selected]);

  return (
    <DayPicker
      mode="single"
      selected={selected}
      onSelect={setSelected}
      style={{ color: styleColor }}
      modifiersClassNames={{
        selected: 'my-selected',
        today: 'my-today'
      }}
    />
  );
}

const getSeries = (data) => {
  return [{
    name: 'Duration',
    data: data
  }];
}


const getOptions = (colorMode) => {
  const foreColor = colorMode == 'dark' ? DARKMODE_FORE_COLOR : LIGHTMODE_FORE_COLOR;
  const toolTipBg = colorMode == 'dark' ? 'dark' : 'light';

  return {
    chart: {
      type: 'area',
      stacked: false,
      foreColor: foreColor,
      zoom: {
        type: 'x',
        enabled: true,
        autoScaleYaxis: true
      },
      toolbar: {
        autoSelected: 'zoom'
      }
    },
    dataLabels: {
      enabled: false
    },
    markers: {
      size: 0,
    },
    fill: {
      type: 'gradient',
      gradient: {
        shadeIntensity: 1,
        inverseColors: false,
        opacityFrom: 0.5,
        opacityTo: 0,
        stops: [0, 90, 100]
      },
    },
    yaxis: {
      labels: {
        formatter: function (val) {
          return (val).toFixed(0);
        },
      },
      title: {
        text: 'Duration'
      },
    },
    xaxis: {
      type: 'datetime',
    },
    tooltip: {
      theme: toolTipBg,
      shared: false,
      y: {
        formatter: function (val) {
          return (val).toFixed(0)
        }
      }
    }
  }
};

function PageTimeSeries() {
  const ctx = useContext(AppContext);
  const [sites, setSites] = useState([]);
  const [measurements, setMeasurements] = useState([]);
  const [day, setDay] = useState(new Date());
  const { height, width } = useWindowDimensions();

  // Get the sites on mount
  useEffect(() => {
    Api.get('sites')
      .then((response) => {
        setSites(response.data);
        if (ctx.selectedSite == "")
          ctx.setSelectedSite(response.data[0].name);
      }).catch((error) => {
        console.log(error);
      })
  }, []);

  useEffect(() => {
    getMeasurements();
  }, [day, ctx.selectedSite]);

  function getMeasurements() {
    if (ctx.selectedSite == "")
      return;
    const nextDay = new Date(day);
    nextDay.setDate(nextDay.getDate() + 1);
    Api.get('measurements/date', {
      params: {
        name: ctx.selectedSite,
        date: { begin: formatDate(day), end: formatDate(nextDay) }
      }
    })
      .then(function (response) {
        const res = response.data.map(o => {
          return { x: o.date.$date, y: o.duration };
        });
        setMeasurements(res);
      })
      .catch(function (error) {
        console.log(error);
      });
  }

  function computeNotSelected(selected) {
    return sites.filter(site => site.name != selected);
  }
  const notSelected = useMemo(
    () => computeNotSelected(ctx.selectedSite), [ctx.selectedSite, sites]
  );

  return (
    <div className='time-wrapper'>
      <Card>
        <div className='m-header'>
          <div className='m-title'>Time Series for <TitleSelect options={notSelected} /></div>
          <div className='m-category'>{measurements.length} Data-points</div>
        </div>
        <div style={{ height: "100%", display: "flex", flexFlow: "column nowrap", justifyContent: "space-between", padding: "10px" }}>
          <div style={{ display: "flex", flexFlow: "row nowrap" }}>
            <ReactApexChart
              style={{ flex: 1 }}
              height={height / 2}
              options={getOptions(ctx.colorMode)}
              series={getSeries(measurements)}
              type="area" />
          </div>
          <div style={{ display: "flex", flexFlow: "row nowrap", justifyContent: "center" }}>
            <MyDayPicker
              onSelect={d => setDay(d)} />
          </div>
        </div>
      </Card>
    </div>
  );
}

export default PageTimeSeries;