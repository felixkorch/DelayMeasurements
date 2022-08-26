// Bootstrap
import Card from 'react-bootstrap/Card';

// Libs
import ReactApexChart from "react-apexcharts";
import { DayPicker } from 'react-day-picker';
import 'react-day-picker/dist/style.css';

// React
import { useState, useEffect, useContext } from 'react';

// Local
import { Api } from '../Api.js'
import { formatDate } from '../scripts/utilities.js';
import { ColorModeContext } from '../components/SideBar.js'

const DARKMODE_FORE_COLOR = '#ffffffc7';
const LIGHTMODE_FORE_COLOR = '#000000';

const MyDayPicker = (props) => {
  const [selected, setSelected] = useState(new Date());
  const colorMode = useContext(ColorModeContext);
  const styleColor = colorMode == 'dark' ? DARKMODE_FORE_COLOR : LIGHTMODE_FORE_COLOR;

  // Calls parent when selected changes
  useEffect(() => {
    props.onSelect(selected);
  }, [selected]);

  return (
    <DayPicker
      mode="single"
      selected={selected}
      onSelect={setSelected}
      style={{color: styleColor}}
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
      height: 500,
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
    title: {
      text: 'Delay over time',
      align: 'left'
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

  const [measurements, setMeasurements] = useState([]);
  const [selected, setSelected] = useState("");
  const [day, setDay] = useState(new Date());
  const [sites, setSites] = useState([]);
  const colorMode = useContext(ColorModeContext);

  // Get the sites on mount
  useEffect(() => {
    Api.get('sites')
      .then((response) => {
        const len = response.data.length;
        setSites(response.data);
        setSelected(len ? response.data[0].name : "");
      }).catch((error) => {
        console.log(error);
      })
  }, []);

  useEffect(() => {
    if (selected == "")
      return;
    getMeasurements();
  }, [day, selected]);

  function getMeasurements() {
    const nextDay = new Date(day);
    nextDay.setDate(nextDay.getDate() + 1);
    Api.get('measurements/date', {
      params: {
        name: selected,
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

  return (
    <div className='time-wrapper'>
      <Card>
        <div style={{ display: "flex", flexFlow: "column nowrap", padding: "10px" }}>
          <div style={{ display: "flex", flexFlow: "row nowrap"}}>
            <ReactApexChart
              style={{flex: 1}}
              options={getOptions(colorMode)}
              series={getSeries(measurements)}
              height="500px"
              type="area" />
          </div>
          <div style={{ display: "flex", flexFlow: "row nowrap",  justifyContent: "center" }}>
            <MyDayPicker
              onSelect={d => setDay(d)} />
          </div>
        </div>
      </Card>
    </div>
  );
}

export default PageTimeSeries;