// React
import { useEffect, useState, useContext } from 'react';

// Local
import { Api } from '../Api.js'
import { ColorModeContext } from '../components/SideBar.js';

// Bootstrap
import Col from 'react-bootstrap/Col';
import Form from 'react-bootstrap/Form';
import Row from 'react-bootstrap/Row';
import Container from 'react-bootstrap/Container';
import Card from 'react-bootstrap/Card';
import Button from 'react-bootstrap/Button';

// Icons
import { BsArrowCounterclockwise, BsFillTrashFill, BsPlus } from "react-icons/bs";
import { AiOutlinePlus } from 'react-icons/ai';

// Libs
import { useFormik } from "formik";
import * as Yup from "yup";

const DARKMODE_INPUT_CLASSES = 'bg-dark text-white-50 border-0';
const LIGHTMODE_INPUT_CLASSES = '';

function UpdateCard(props) {

  const colorMode = useContext(ColorModeContext);
  const inputStyleClasses = colorMode == 'dark' ? DARKMODE_INPUT_CLASSES : LIGHTMODE_INPUT_CLASSES;

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
    <div style={{ padding: "10px" }}>
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
          <Col md={12} lg={3}>
            <Button variant="primary" onClick={formik.handleSubmit} > <BsArrowCounterclockwise /> </Button>
            <Button variant="danger" onClick={props.onDelete} style={{ marginLeft: "5px" }}> <BsFillTrashFill /> </Button>
          </Col>
        </Row>

      </Container>
    </div>
  )
}

function PageModify() {

  const [siteList, setSiteList] = useState(null);
  const [isCreating, setIsCreating] = useState(false);

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
    setIsCreating(false);
    Api.post('sites', newObj)
      .then(function (response) {
        const id = response.data.id
        const newState = [...siteList, { _id: id, ...newObj }];
        setSiteList(newState);
      })
      .catch(function (error) {
        console.log(error);
      });
  }

  return (
    <div className="page-modify-wrapper">
      <Container fluid>
        <Card>
            {siteList && siteList.map((site, index) => {
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
            className='add-site-button'
            variant="primary"
            onClick={handleNewItem}
            style={{ width: "42px", margin: "20px" }}>
            <AiOutlinePlus />
          </Button>
        </Card>
      </Container>
    </div>
  );
}

export default PageModify;