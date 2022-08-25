import Container from 'react-bootstrap/Container';
import Row from 'react-bootstrap/Row';
import Col from 'react-bootstrap/Col';
import ReactApexChart from "react-apexcharts";
import { DayPicker } from 'react-day-picker';
import 'react-day-picker/dist/style.css';

import { Api } from '../Api.js'
import { useAfterFirstRender, formatDate } from '../scripts/utilities.js';
import { useState, useEffect } from 'react';

const MyDayPicker = (props) => {
  const [selected, setSelected] = useState(new Date());

  // Calls parent when selected changes
  useAfterFirstRender(() => {
    props.onSelect(selected);
  }, [selected]);

  return (
    <DayPicker
      mode="single"
      selected={selected}
      onSelect={setSelected}
    />
  );
}

const getSeries = (data) => {
  return [{
    name: 'Duration',
    data: data
  }];
}

const getOptions = () => {
  return {
    chart: {
      type: 'area',
      stacked: false,
      height: 500,
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
  const [day, setDay] = useState(null);
  const [sites, setSites] = useState([]);

  // Get the sites on mount
  useEffect(() => {
    Api.get('sites')
      .then((response) => {
        const len = response.data.length;
        console.log(len);
        setSites(response.data);
        setSelected(len ? response.data[0].name : "");
      }).catch((error) => {
        console.log(error);
      })
  }, []);

  useAfterFirstRender(() => {
    if (selected == "")
      return;
    getMeasurements();
  }, [day]);

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
    <div className='stats-wrapper'>
      <Container fluid>
        <Row>
          <Col xs={12}>
            <ReactApexChart options={getOptions()} series={getSeries(measurements)} type="area" height={500} />
          </Col>
        </Row>
        <Row>
          <Col xs={{ span: 4, offset: 4 }}>
            <MyDayPicker onSelect={d => setDay(d)} />
          </Col>
        </Row>
      </Container>
    </div>
  );
}

export default PageTimeSeries;