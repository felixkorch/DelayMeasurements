import React from "react";
import './App.css';
import './Dark.css'
import PageMeasurements from './pages/PageMeasurements.js'
import PageModify from './pages/PageModify.js'
import PageTimeSeries from './pages/PageTimeSeries'

import SideBar from './components/SideBar.js'
import { ColorModeContext } from "./components/SideBar.js";
import { Route, Routes, BrowserRouter, Navigate } from "react-router-dom";
import { useState } from 'react';

function App() {

  const [colorMode, setColorMode] = useState("dark");
  const appClassName = colorMode == 'dark' ? 'App dark' : 'App';

  return (
    <ColorModeContext.Provider value={colorMode}>
      <div className={appClassName}>
        <BrowserRouter>
          <SideBar setColorMode={(color) => setColorMode(color) } />
          <div className="main">
            <Routes>
              <Route path="/" element={<Navigate to="/measurements" />} />
              <Route path="/measurements" element={<PageMeasurements />} />
              <Route path="/series" element={<PageTimeSeries />} />
              <Route path="/modify" element={<PageModify />} />
            </Routes>
          </div>
        </BrowserRouter>
      </div>
    </ColorModeContext.Provider>
  );
}

export default App;
