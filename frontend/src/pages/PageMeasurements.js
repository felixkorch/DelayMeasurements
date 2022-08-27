// Bootstrap
import Button from 'react-bootstrap/Button';
import Card from 'react-bootstrap/Card';

// React
import { useEffect, useState, useRef, useMemo, useCallback, useContext } from 'react';
import { useAfterFirstRender } from '../scripts/utilities.js';

// Local
import { Api } from '../Api.js'
import { ColorModeContext } from '../components/SideBar.js';

// Libs
import { AgGridReact } from 'ag-grid-react'; // the AG Grid React Component
import 'ag-grid-community/styles/ag-grid.css'; // Core grid CSS, always needed
import 'ag-grid-community/styles/ag-theme-alpine.css'; // Optional theme CSS

function MeasurementsTable(props) {

  const gridRef = useRef();
  const colorMode = useContext(ColorModeContext);
  const tableTheme = colorMode == 'dark' ? 'ag-theme-alpine-dark' : 'ag-theme-alpine';

  const onBtnExport = useCallback(() => {
    gridRef.current.api.exportDataAsCsv();
  }, []);

  // Each Column Definition results in one Column.
  const [columnDefs, setColumnDefs] = useState([
    { field: 'date', filter: 'agDateColumnFilter' },
    { field: 'duration', filter: 'agNumberColumnFilter' },
    { field: 'code', filter: 'agNumberColumnFilter' },
    { field: 'size', filter: 'agNumberColumnFilter' }
  ]);

  // DefaultColDef sets props common to all Columns
  const defaultColDef = useMemo(() => ({
    sortable: true,
    flex: 1,
  }));


  return (
    <Card style={{ height: "100%" }}>
      <div className='m-header'>
        <div className='m-title'>Measurements for {props.selected}</div>
        <div className='m-category'>{props.data.length} Data-points</div>
      </div>
      <Card.Body>
        <div className={tableTheme} style={{ height: "90%" }}>
          <AgGridReact
            rowData={props.data} // Row Data for Rows
            ref={gridRef}
            columnDefs={columnDefs} // Column Defs for Columns
            defaultColDef={defaultColDef} // Default Column Properties
            animateRows={true} // Optional - set to 'true' to have rows animate when sorted
            rowSelection='multiple' // Options - allows click selection of rows
          />
        </div>
        <Button
          variant="primary"
          onClick={onBtnExport}
          style={{ fontSize: "14px", float: "right", marginTop: "20px" }}>
          Download CSV export file
        </Button>

      </Card.Body>
    </Card>
  );
}

function PageMeasurements() {

  const [measurements, setMeasurements] = useState([]);
  const [selected, setSelected] = useState("");
  const [sites, setSites] = useState([]);

  useEffect(() => {
    const interval = setInterval(() => {
      if (selected == "")
        return;
      getMeasurements();
    }, 1000);

    return () => clearInterval(interval);
  }, []);

  useAfterFirstRender(() => {
    if (selected == "")
      return;
    getMeasurements();
  }, [selected]);

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

  function getMeasurements() {
    Api.get('measurements', { params: { name: selected, limit: 1000 } })
      .then(function (response) {
        const res = response.data.map((e, key) => {
          return { ...e, id: key, date: new Date(e.date.$date) };
        });
        setMeasurements(prev => res);
      })
      .catch(function (error) {
        console.log(error);
      });
  }

  return (
    <div className='stats-wrapper'>
      <MeasurementsTable data={measurements} selected={selected} />
    </div>
  );
}

export default PageMeasurements;