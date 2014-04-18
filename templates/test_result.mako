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
      <p><b>Result:</b> ${value.get('status', 'None')}</p>
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
        <td>${value.get('returncode', 'None')}</td>
      </tr>
      <tr>
        <td>Time</td>
        <td>${value.get('time', 'None')}</b>
      </tr>
    ## Info is deprecated and may disapear someday
    % if value.get('info') is not None:
      <tr>
        <td>Info</td>
        <td>
          <pre>${value.get('info') | h}</pre>
        </td>
      </tr>
    % endif
      <tr>
        <td>Stdout</td>
        <td>
          <pre>${value.get('out', 'None') | h}</pre>
        </td>
      </tr>
      <tr>
        <td>Stderr</td>
        <td>
          <pre>${value.get('err', 'None') | h}</pre>
        </td>
      </tr>
    % if value.get('env') is not None:
      <tr>
        <td>Environment</td>
        <td>
          <pre>${value.get('env') | h}</pre>
        </td>
      </tr>
    % endif
      <tr>
        <td>Command</td>
        <td>
          </pre>${value.get('command', 'None')}</pre>
        </td>
      </tr>
    % if value.get('traceback') is not None:
      <tr>
        <td>Traceback</td>
        <td>
          <pre>${value.get('traceback') | h}</pre>
        </td>
      </tr>
    % endif
      <tr>
        <td>dmesg</td>
        <td>
          <pre>${value.get('dmesg', 'None') | h}</pre>
        </td>
      </tr>
    </table>
    <p><a href="${index}">Back to summary</a></p>
  </body>
</html>
