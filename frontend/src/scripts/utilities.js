import { useEffect, useState, useRef } from 'react';

// Returns a string with format: %Y-%m-%d
export function formatDate(date) {
    var d = new Date(date),
      month = '' + (d.getMonth() + 1),
      day = '' + d.getDate(),
      year = d.getFullYear();
  
    if (month.length < 2)
      month = '0' + month;
    if (day.length < 2)
      day = '0' + day;
  
    return [year, month, day].join('-');
  }
  
  // Returns a (ready to print) string from a date
  export function hoursAndMinutes(date) {
    const days = ["Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"];
    const withPmAm = date.toLocaleTimeString('en-US', {
      // en-US can be set to 'default' to use user's browser settings
      hour: '2-digit',
      minute: '2-digit',
  
    });
    return days[date.getDay()] + " " + withPmAm;
  }

  /***  Custom Hooks  ****/
  export const useAfterFirstRender = (func, deps) => {
    const didMount = useRef(false);
  
    useEffect(() => {
      if (didMount.current) {
        func();
      } else {
        didMount.current = true;
      }
    }, deps);
  };