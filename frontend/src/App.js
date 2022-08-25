import React from "react";
import './App.css';
import './dark.css'
import PageMeasurements from './pages/PageMeasurements.js'
import PageModify from './pages/PageModify.js'
import PageTimeSeries from './pages/PageTimeSeries'

import SideBar from './components/SideBar.js'
import { Route, Routes, BrowserRouter, Navigate } from "react-router-dom";

function App() {
  return (
    <div className="App">
      <BrowserRouter>
        <SideBar />
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
  );
}

export default App;
