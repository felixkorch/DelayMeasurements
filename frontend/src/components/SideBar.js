import { Form } from "react-bootstrap";
import { NavLink } from "react-router-dom";
import { BsFileEarmarkPlus, BsBarChart, BsCloudArrowDown, BsClipboardData } from "react-icons/bs";
import { TbTimeline } from 'react-icons/tb'

const routes = [
  { "path": "/measurements", "name": "Measurements", "icon": <BsClipboardData className="side-icon" /> },
  { "path": "/series", "name": "Time Series", "icon": <TbTimeline className="side-icon" /> },
  { "path": "/modify", "name": "Modify", "icon": <BsFileEarmarkPlus className="side-icon" /> }
]

function SideBar() {

  return (
    <div className="sidebar-wrapper">
      <div className="sidebar-header">
      <div className="sidebar-title">Delay Measurements</div>
      </div>
      <hr className="sidebar-divider" />
      {routes.map((route, key) => {
        return (
          <div key={key} className="sidebar-li">
            <NavLink to={route.path} className="nav-link">
              {route.icon}
              <div className="route-text">{route.name}</div>
            </NavLink>
          </div>
        )
      })}
    </div>
  );

}

export default SideBar;