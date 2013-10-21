<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//END"
 "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
  <head>
    <meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
    <title>${testname} - Details</title>
    <link rel="stylesheet" href="${css}" type="text/css" />
  </head>
  <body>
    <h1>Results for ${testname}</h1>
    <h2>Overview</h2>
    <div>
      <p><b>Result:</b> ${status}</p>
    </div>
    <p><a href="${index}">Back to summary</a></p>
    <h2>Details</h2>
    <table>
      <tr>
        <th>Detail</th>
        <th>Value</th>
      </tr>
      <tr>
        <td>Returncode</td>
        <td>${returncode}</td>
      </tr>
      <tr>
        <td>Time</td>
        <td>${time}</b>
      </tr>
      <tr>
        <td>Info</td>
        <td>
          <pre>${info | h}</pre>
        </td>
      </tr>
      % if env:
      <tr>
        <td>Environment</td>
        <td>
          <pre>${env | h}</pre>
        </td>
      </tr>
      % endif
      <tr>
        <td>Command</td>
        <td>
          </pre>${command}</pre>
        </td>
      </tr>
      <tr>
        <td>Traceback</td>
        <td>
          <pre>${traceback | h}</pre>
        </td>
      </tr>
      <tr>
        <td>dmesg</td>
        <td>
          <pre>${dmesg | h}</pre>
        </td>
      </tr>
    </table>
    <p><a href="${index}">Back to summary</a></p>
  </body>
</html>
