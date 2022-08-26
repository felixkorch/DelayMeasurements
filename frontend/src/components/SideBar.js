// Bootstrap
import Button from 'react-bootstrap/Button';

// React
import { NavLink } from "react-router-dom";
import { createContext, useContext } from "react";

// Icons
import { BsFileEarmarkPlus, BsBarChart, BsCloudArrowDown, BsClipboardData } from "react-icons/bs";
import { TbTimeline } from 'react-icons/tb'
import { MdLightMode, MdDarkMode } from 'react-icons/md'

const routes = [
  { "path": "/measurements", "name": "Measurements", "icon": <BsClipboardData className="side-icon" /> },
  { "path": "/series", "name": "Time Series", "icon": <TbTimeline className="side-icon" /> },
  { "path": "/modify", "name": "Modify", "icon": <BsFileEarmarkPlus className="side-icon" /> }
]

export const ColorModeContext = createContext(undefined);

// Hook to provide access to context object
export const UseColorModeContext = () => {
  return useContext(ColorModeContext);
};

function SideBar(props) {

  return (
    <div className="sidebar-wrapper">
      <div className='sidebar-content'>
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
      <div className='sidebar-colormode'>
        <Button
          onClick={() => props.setColorMode("light")}
          size="sm"
          variant="light"
          style={{ marginRight: '10px' }}>
          <MdLightMode />
        </Button>
        <Button
          onClick={() => props.setColorMode("dark")}
          size="sm"
          variant="dark">
          <MdDarkMode />
        </Button>
      </div>
    </div>
  );

}

export default SideBar;