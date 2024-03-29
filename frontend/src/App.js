// React
import { React, useState, createContext } from 'react';
import { Route, Routes, BrowserRouter, Navigate } from "react-router-dom";

// Local
import './App.css';
import './dark.css'
import PageMeasurements from './pages/PageMeasurements.js'
import PageModify from './pages/PageModify.js'
import PageTimeSeries from './pages/PageTimeSeries'
import SideBar from './components/SideBar.js'


export const AppContext = createContext({
  colorMode: "dark",
  selectedSite: "",
  setColorMode: () => {},
  setSelectedSite: () => {}
});


function App() {

  const [colorMode, setColorMode] = useState("dark");
  const [selectedSite, setSelectedSite] = useState("");
  const value = { colorMode, selectedSite, setColorMode, setSelectedSite };

  const appClassName = colorMode == 'dark' ? 'App dark' : 'App';

  return (
    <AppContext.Provider value={value}>
      <div className={appClassName}>
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
    </AppContext.Provider>
  );
}

export default App;
