/*
This is a Node.js server that receives a GET requests with some data and appends it to a file.
*/
const { json } = require('body-parser');
const moment = require('moment-timezone');
const express = require('express');
const fs = require('fs');

const app = express();

app.get('/append', (req, res) => {
  const data = req.query;
  console.log('Data received:', data);
  if (!data) {
    return res.status(400).send('No data provided');
  }
  if(data.pepe && data.papo) {
    // Generate timestamp
    const now = moment().tz('Europe/Madrid');
    const timestamp = now.format('YYYY-MM-DD HH:mm:ss');
    const dataToWrite = JSON.stringify( {timestamp, data} );
    // Append data to file
    fs.appendFile('data.txt', `${dataToWrite}\n`, (err) => {
      if (err) {
        console.error(err);
        return res.status(500).send('Error writing to file');
      }
  
      console.log(`Data "${dataToWrite}" appended to file`);
      res.send(`Data appended to file at ${timestamp}`);
    });
  } else {
    res.send('Wrong data format');
  }

});

app.get('/dump', (req, res) => {
  fs.readFile('data.txt', 'utf8', (err, data) => {
    data = "<pre>" + data + "</pre>";
    if (err) {
      console.error(err);
      return res.status(500).send('Error reading file');
    }

    console.log('File contents:', data);
    res.send(data);
  });

});

var port = process.env.PORT || 3000;
app.listen(port, () => {
  console.log('Server listening on port 3000');
});
