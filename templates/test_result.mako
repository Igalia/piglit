<!DOCTYPE html>
<html>
  <head>
    <meta charset="UTF-8">
    <title>${testname} - Details</title>
    <link rel="stylesheet" href="${css}">
  </head>
  <body>
    <h1>Results for ${testname}</h1>
    <h2>Overview</h2>
    <div>
      <p><b>Result:</b> ${value.result}</p>
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
        <td>${value.returncode}</td>
      </tr>
      <tr>
        <td>Time</td>
        <td>${value.time.delta}</td>
      </tr>
    % if value.images:
      <tr>
        <td>Images</td>
        <td>
          <table>
            <tr>
              <td/>
              <td>reference</td>
              <td>rendered</td>
            </tr>
          % for image in images:
            <tr>
              <td>${image['image_desc']}</td>
              <td><img src="file://${image['image_ref']}" /></td>
              <td><img src="file://${image['image_render']}" /></td>
            </tr>
          % endfor
          </table>
        </td>
      </tr>
    % endif
      <tr>
        <td>Stdout</td>
        <td>
          <pre>${value.out | h}</pre>
        </td>
      </tr>
      <tr>
        <td>Stderr</td>
        <td>
          <pre>${value.err | h}</pre>
        </td>
      </tr>
    % if value.environment:
      <tr>
        <td>Environment</td>
        <td>
          <pre>${value.environment | h}</pre>
        </td>
      </tr>
    % endif
      <tr>
        <td>Command</td>
        <td>
          <pre>${value.command}</pre>
        </td>
      </tr>
    % if value.exception:
      <tr>
        <td>Exception</td>
        <td>
          <pre>${value.exception | h}</pre>
        </td>
      </tr>
    % endif
    % if value.traceback:
      <tr>
        <td>Traceback</td>
        <td>
          <pre>${value.traceback | h}</pre>
        </td>
      </tr>
    % endif
      <tr>
        <td>dmesg</td>
        <td>
          <pre>${value.dmesg | h}</pre>
        </td>
      </tr>
    </table>
    <p><a href="${index}">Back to summary</a></p>
  </body>
</html>
