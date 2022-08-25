import axios from 'axios'

var backend = process.env.REACT_APP_BACKEND || 'localhost'
var port = process.env.REACT_APP_BACKEND_PORT || '5000'
var protocol = process.env.REACT_APP_BACKEND_PROTOCOL || 'http'


var backendApiEndpoint = protocol + '://' + backend + ':' + port + '/api'
console.log('Using backend endpoint: ' + backendApiEndpoint)

export const Api = axios.create({
  baseURL: backendApiEndpoint
})

