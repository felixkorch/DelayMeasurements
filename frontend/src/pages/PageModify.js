// React
import { useEffect, useState, useContext } from 'react';

// Local
import { Api } from '../Api.js'
import { AppContext } from '../App.js';

// Bootstrap
import Col from 'react-bootstrap/Col';
import Form from 'react-bootstrap/Form';
import Row from 'react-bootstrap/Row';
import Container from 'react-bootstrap/Container';
import Card from 'react-bootstrap/Card';
import Button from 'react-bootstrap/Button';
import Alert from 'react-bootstrap/Alert';

// Icons
import { BsArrowCounterclockwise, BsFillTrashFill, BsPlus } from "react-icons/bs";
import { AiOutlinePlus } from 'react-icons/ai';

// Libs
import { useFormik } from "formik";
import * as Yup from "yup";

const DARKMODE_INPUT_CLASSES = 'bg-dark text-white-50 border-0';
const LIGHTMODE_INPUT_CLASSES = '';

function UpdateCard(props) {

  const ctx = useContext(AppContext);
  const inputStyleClasses = ctx.colorMode == 'dark' ? DARKMODE_INPUT_CLASSES : LIGHTMODE_INPUT_CLASSES;

  const UpdateSchema = Yup.object().shape({
    name: Yup.string()
      .required('Name is a required field.'),
    url: Yup.string()
      .url('Url not correctly formatted.')
      .required('Url is a required field.'),
    interval: Yup.number().min(1000, 'Interval has to be at least 1000ms').required('Required'),
  });

  const formik = useFormik({
    initialValues: {
      name: props.name,
      url: props.url,
      interval: props.interval
    },
    validationSchema: UpdateSchema,
    onSubmit: (values) => {
      if (formik.isValid)
        props.onUpdate(values);
    },
    validateOnChange: true,
    validateOnBlur: false
  });

  function changeHandler(e) {
    formik.handleChange(e);
  }

  return (
    <div style={{ width: "100%" }}>
      <Container fluid>
        <Row>
          <Col md={12} lg={3} >
            <Form.Control
              className={inputStyleClasses}
              isInvalid={formik.errors.name}
              id="name"
              placeholder="Site name"
              defaultValue={props.name}
              onChange={changeHandler} />
            <div className="site-modify-error">{formik.errors.name}</div>
          </Col>
          <Col md={12} lg={4}>
            <Form.Control
              className={inputStyleClasses}
              isInvalid={formik.errors.url}
              id="url"
              placeholder="Url"
              defaultValue={props.url}
              onChange={changeHandler} />
            <div className="site-modify-error">{formik.errors.url}</div>
          </Col>
          <Col md={12} lg={2}>
            <Form.Control
              className={inputStyleClasses}
              isInvalid={formik.errors.interval}
              id="interval"
              placeholder="Interval"
              defaultValue={props.interval}
              onChange={changeHandler} />
            <div className="site-modify-error">{formik.errors.interval}</div>
          </Col>
          <Col md={12} lg={2} className='d-flex justify-content-center'>
            <Button variant="primary" onClick={formik.handleSubmit} style={{ height: "41px" }}> <BsArrowCounterclockwise /> </Button>
            <Button variant="danger" onClick={props.onDelete} style={{ marginLeft: "5px", height: "41px" }}> <BsFillTrashFill /> </Button>
          </Col>
        </Row>

      </Container>
      <hr></hr>
    </div>
  )
}

function PageModify() {

  const [siteList, setSiteList] = useState(null);
  const [isCreating, setIsCreating] = useState(false);
  const [showAlert, setShowAlert] = useState(false);
  const [alertContent, setAlertContent] = useState("Success!");

  // Get the sites on mount
  useEffect(() => {
    Api.get('sites')
      .then(function (response) {
        setSiteList(() => response.data);
      })
      .catch(function (error) {
        console.log(error);
      });
  }, []);

  function handleChange(id, newObj) {
    const newState = siteList.map((obj) => {
      // If id equals index, update
      if (obj._id.$oid === id) {
        return { _id: obj._id, name: newObj.name, url: newObj.url, interval: newObj.interval };
      }
      // Otherwise return object as is
      return obj;
    });
    setSiteList(newState);
  }

  function handleUpdate(id, newObj) {
    Api.put('sites/' + id,
      { _id: { $oid: id }, id, name: newObj.name, url: newObj.url, interval: newObj.interval })
      .catch(function (error) {
        console.log(error);
      });
    handleChange(id, newObj);
  }

  function handleDelete(id) {
    Api.delete('sites/' + id)
      .catch(function (error) {
        console.log(error);
      });

    const newState = siteList.filter((o) => o._id.$oid != id);
    setSiteList(newState);
  }


  function handleNewItem() {
    setIsCreating(!isCreating);
  }

  function handleAdd(newObj) {
    Api.post('sites', newObj)
      .then(async function (response) {
        const id = response.data.id
        const newState = [...siteList, { _id: id, ...newObj }];
        await setSiteList(newState);
        setIsCreating(false);
        setShowAlert(true);
        setAlertContent(<div>Now monitoring <b>{newObj.name}</b>!</div>);
      })
      .catch(function (error) {
        console.log(error);
      });
  }

  // Set timeout for alert when:
  // showAlert false -> true
  useEffect(() => {
    if (showAlert == false)
      return;
    const doCloseAlert = setTimeout(() => setShowAlert(false), 5000);
    return () => clearTimeout(doCloseAlert);
  }, [showAlert])


  return (
    <div className="page-modify-wrapper">
      <Container fluid>
        <Alert variant="success" dismissible transition show={showAlert} onClose={() => setShowAlert(false)}>
          {alertContent}
        </Alert>
        <Card style={{ alignItems: "center", padding: "15px 10px" }}>
            {siteList && siteList.map(site => {
              return <UpdateCard
                key={site._id.$oid}
                name={site.name}
                url={site.url}
                interval={site.interval}
                onUpdate={(o) => handleUpdate(site._id.$oid, o)}
                onDelete={() => handleDelete(site._id.$oid)}
              />
            })}

            {isCreating ?
              <UpdateCard
                name=""
                url=""
                interval=""
                onUpdate={(o) => handleAdd(o)}
                onDelete={() => setIsCreating(!isCreating)}
              /> : null
            }
          <Button
            className='add-site-button rounded-circle'
            variant="primary"
            onClick={handleNewItem}
            style={{ marginRight: "auto" }}>
            <AiOutlinePlus />
          </Button>
        </Card>
      </Container>
    </div>
  );
}

export default PageModify;