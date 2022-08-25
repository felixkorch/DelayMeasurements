import Container from 'react-bootstrap/Container';
import Row from 'react-bootstrap/Row';
import Col from 'react-bootstrap/Col';
import Card from 'react-bootstrap/Card';
import Form from 'react-bootstrap/Form';
import Table from 'react-bootstrap/Table';
import { BiSortAlt2 } from 'react-icons/bi'
import { IoFilter } from 'react-icons/io5'
import { CgCloseR } from 'react-icons/cg'
import { useEffect, useState, useRef, useMemo, useContext } from 'react';
import { useAfterFirstRender } from '../scripts/utilities.js';
import { Api } from '../Api.js'

function Sort(props) {

  function handleClick() {
    props.onSort();
  }

  return (
    <BiSortAlt2 style={{ cursor: "pointer" }} onClick={handleClick} />
  );
}

function useOutsideAlerter(btnRef, contentRef, onClose) {
  useEffect(() => {
    function handleClickOutside(event) {
      const buttonClick = btnRef.current && btnRef.current.contains(event.target);
      const contentClick = contentRef.current && contentRef.current.contains(event.target);
      if (buttonClick || contentClick)
        return;
      onClose();
    }
    // Bind the event listener
    document.addEventListener("mousedown", handleClickOutside);
    return () => {
      // Unbind the event listener on clean up
      document.removeEventListener("mousedown", handleClickOutside);
    };
  }, [btnRef, contentRef]);
}

function FilterDate(props) {
  const handleChange = (e) => {
    props.onFilter(new Date(e.target.value));
  };

  return (
    <div className='filter-date'>
    <Form.Control type="date" onChange={handleChange} />
    </div>
  );
}

function FilterDuration(props) {
  const [operator, setOperator] = useState("");
  const [threshold, setThreshold] = useState(0);

  const delay = ms => new Promise(
    resolve => setTimeout(resolve, ms)
  );

  const handleThresholdChange = async (e) => {
    setThreshold(e.target.value)
    await delay(1000);
    props.onFilter({ operator: operator, duration: e.target.value });
  };

  const handleOperatorChange = async (e) => {
    setOperator(e.target.value)
    await delay(1000);
    props.onFilter({ operator: e.target.value, duration: threshold });
  };

  return (
    <div className='filter-duration'>
      <Form.Select value={operator} onChange={handleOperatorChange}>
        <option>LTE</option>
        <option>GTE</option>
        <option>EQ</option>
      </Form.Select>
      <Form.Control value={threshold} type="number" onChange={handleThresholdChange} />
    </div>
  );
}


function Filter(props) {
  const btnRef = useRef(null);
  const contentRef = useRef(null);
  const [show, setShow] = useState(false);
  useOutsideAlerter(btnRef, contentRef, () => { setShow(false) });

  let className = show ? 'filter-content show' : 'filter-content';

  return (
    <div className='filter' ref={btnRef}>
      <IoFilter onClick={() => setShow(!show)} style={{ cursor: "pointer" }} />
      <div ref={contentRef} className={className}>
        <Card className='filter-body'>
          <Card.Body>
            {props.content}
          </Card.Body>
        </Card>
      </div>
    </div>
  );
}


function MeasurementsTable(props) {

  const [dateFilter, setDateFilter] = useState(null);
  const [durationFilter, setDurationFilter] = useState(null);
  const [sortBy, setSortBy] = useState(0); // 0: No sorting, 1: date, 2: duration

  const filteredData = useMemo(() => {
    let currentFilter = props.data;
    if (dateFilter)
      currentFilter = dateFilter.fn(currentFilter);
    if (durationFilter)
      currentFilter = durationFilter.fn(currentFilter);
    switch (sortBy) {
      case 0: return currentFilter;
      case 1: currentFilter.sort((a, b) => b.date - a.date); break;
      case 2: currentFilter.sort((a, b) => b.duration - a.duration); break;
    }
    return currentFilter;
  }, [dateFilter, durationFilter, sortBy, props.data]);

  function filterDate(list, decision) {
    return list.filter(e => {
      const operand1 = e.date;
      const operand2 = decision;
      return operand1.getDate() === operand2.getDate();
    });
  }

  function filterDuration(list, decision) {
    return list.filter(e => {
      const operand1 = e.duration;
      const operand2 = decision.duration;
      switch (decision.operator) {
        case "LTE": return operand1 <= operand2;
        case "GTE": return operand1 >= operand2;
        case "EQ": return operand1 == operand2;
      }
    });
  }

  function doFilterData(decision) {
    const fn = (list) => filterDate(list, decision);
    const data = { fn: fn, decision: decision };
    setDateFilter(data);
  }

  function doFilterDuration(decision) {
    const fn = (list) => filterDuration(list, decision);
    setDurationFilter({ fn: fn, decision: decision });
  }

  return (
    <Card>
      <div className='m-header'>
        <div className='m-title'>Measurements for {props.selected}</div>
        <div className='m-category'>{props.data.length} Data-points</div>
      </div>
      <Card.Body>
        <div className='active-filters'>
          {dateFilter ? <div className='active-filter'>
            {dateFilter.decision.toLocaleDateString()}
            <CgCloseR onClick={() => setDateFilter(null)} style={{ "marginLeft": "5px", "cursor": "pointer" }} />
          </div> : null}
          {durationFilter ? <div className='active-filter'>
            {durationFilter.decision.operator} {durationFilter.decision.duration}
            <CgCloseR onClick={() => setDurationFilter(null)} style={{ "marginLeft": "5px", "cursor": "pointer" }} />
          </div> : null}
        </div>
        <Table striped bordered hover>
          <thead>
            <tr>
              <th>
                <div className='table-head'>
                  <div>Date <Sort onSort={() => setSortBy(1)} /></div>
                  <Filter content={<FilterDate onFilter={doFilterData} />} />
                </div>
              </th>
              <th>
                <div className='table-head'>
                  <div>Duration <Sort onSort={() => setSortBy(2)} /></div>
                  <Filter content={<FilterDuration onFilter={doFilterDuration} />} />
                </div>
              </th>
              <th>
                <div className='table-head'>
                <div>Code <Sort /></div>
                  <Filter />
                </div>
              </th>
              <th>
                <div className='table-head'>
                <div>Size <Sort /></div>
                  <Filter />
                </div>
              </th>
            </tr>
          </thead>
          <tbody>
            {filteredData?.map(e => {
              return (
                <tr key={e.id}>
                  <td>{e.date.toLocaleString()}</td>
                  <td>{e.duration}</td>
                  <td>{e.code}</td>
                  <td>{e.size}</td>
                </tr>
              )
            })}
          </tbody>
        </Table>
      </Card.Body>
    </Card>
  );
}

function PageMeasurements() {

  const [measurements, setMeasurements] = useState([]);
  const [selected, setSelected] = useState("");
  const [sites, setSites] = useState([]);

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
        console.log(len);
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
          <MeasurementsTable data={measurements} selected={selected} />
        </Row>
      </Container>
    </div>
  );
}

export default PageMeasurements;