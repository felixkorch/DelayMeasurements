// Bootstrap
import Button from 'react-bootstrap/Button';
import Card from 'react-bootstrap/Card';

// React
import { useEffect, useState, useRef, useMemo, useCallback, useContext } from 'react';
import { useAfterFirstRender } from '../scripts/utilities.js';

// Local
import { Api } from '../Api.js'
import { AppContext } from '../App.js';

// Libs
import { AgGridReact } from 'ag-grid-react'; // the AG Grid React Component
import 'ag-grid-community/styles/ag-grid.css'; // Core grid CSS, always needed
import 'ag-grid-community/styles/ag-theme-alpine.css'; // Optional theme CSS
import TitleSelect from '../components/TitleSelect.js';

function MeasurementsTable(props) {

  const gridRef = useRef();
  const ctx = useContext(AppContext);
  const tableTheme = ctx.colorMode == 'dark' ? 'ag-theme-alpine-dark' : 'ag-theme-alpine';

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
    <Card>
      <div className='m-header'>
        <div className='m-title'>Measurements for <TitleSelect /></div>
        <div className='m-category'>{props.data.length} Data-points</div>
      </div>
      <Card.Body>
        <div style={{ display: "flex", flexDirection: "column", height: "700px" }}>
          <div className={tableTheme} style={{ flex: "1"}}>
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
            style={{ fontSize: "14px", float: "right", height: "35px", width: "200px", marginTop: "20px", marginLeft: "auto" }}>
            Download CSV export file
          </Button>
        </div>
      </Card.Body>
    </Card>
  );
}

function PageMeasurements() {
  const ctx = useContext(AppContext);
  const [measurements, setMeasurements] = useState([]);
  const [timer, setTimer] = useState(0);

  // Get measurements every second
  /* TODO
  useEffect(() => {
    const interval = setInterval(async () => {
      setTimer(prev => prev + 1);
      if (ctx.selectedSite == "")
        return;
      getMeasurements();
    }, 1000);

    return () => clearInterval(interval);
  }, [timer]); */

  // Get measurements when selected site changes
  useEffect(() => {
    getMeasurements();
  }, [ctx.selectedSite]);

  function getMeasurements() {
    if (ctx.selectedSite == "")
      return;

    Api.get('measurements', { params: { name: ctx.selectedSite, limit: 1000 } })
      .then(function (response) {
        const res = response.data.map((e, key) => {
          return { ...e, id: key, date: new Date(e.date.$date) };
        });
        setMeasurements(() => res);
      })
      .catch(function (error) {
        console.log(error);
      });
  }

  return (
    <div className='stats-wrapper'>
      <MeasurementsTable data={measurements} />
    </div>
  );
}

export default PageMeasurements;