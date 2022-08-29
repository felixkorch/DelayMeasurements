// Icons
import { RiArrowDropDownLine } from 'react-icons/ri';

// React
import { useEffect, useState, useRef, useContext, useMemo } from 'react';

// Local
import { AppContext } from '../App.js';
import { Api } from '../Api.js';

// Bootstrap
import ListGroup from 'react-bootstrap/ListGroup';

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

export default function TitleSelect() {
  const ctx = useContext(AppContext);
  const btnRef = useRef(null);
  const contentRef = useRef(null);
  const [show, setShow] = useState(false);
  const [sites, setSites] = useState([]);
  useOutsideAlerter(btnRef, contentRef, () => { setShow(false) });

  const className = show ? 'title-select-dropdown show' : 'title-select-dropdown';

  // Get the sites on mount
  useEffect(() => {
    Api.get('sites')
      .then((response) => {
        setSites(response.data);
        if(ctx.selectedSite == "")
          ctx.setSelectedSite(response.data[0].name);
      }).catch((error) => {
        console.log(error);
      })
  }, []);

  function computeNotSelected(selected) {
    return sites.filter(site => site.name != selected);
  }
  const notSelected = useMemo(
    () => computeNotSelected(ctx.selectedSite), [ctx.selectedSite, sites]
  );

  return (
    <div className='title-select'>
      <div className='title-select-click' ref={btnRef} onClick={() => setShow(!show)}>
        {ctx.selectedSite} <RiArrowDropDownLine />
      </div>
      <div ref={contentRef} className={className} style={{ marginTop: "5px" }}>
        <ListGroup style={{ borderRadius: "10px" }}>
          {notSelected?.map((option, id) => {
            return <ListGroup.Item key={id} className="title-select-li">
              <div
                style={{ cursor: "pointer", display: "inline-block", userSelect: "none" }}
                onClick={() => ctx.setSelectedSite(option.name)}>
                {option.name}
              </div>
            </ListGroup.Item>;
          })}
        </ListGroup>
      </div>
    </div>
  )
}