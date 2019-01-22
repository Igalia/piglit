<%!
  import six
%>
<!DOCTYPE html>
<html>
  <head>
    <meta charset="UTF-8">
    <title>${name} - System info</title>
    <link rel="stylesheet" href="../result.css">
  </head>
  <body>
    <h1>System info for ${name}</h1>
    <p>
      <a href="../index.html">Back to summary</a>
    </p>
    <table>
      <tr>
        <th>Detail</th>
        <th>Value</th>
      </tr>
      <tr>
        <td>totals</td>
        <td>
          <table>
            % for key, value in sorted(six.iteritems(totals), key=lambda i: (i[1], i[0]), reverse=True):
            <tr><td>${key}</td><td>${value}</td></tr>
            % endfor
            <tr><td>total</td><td>${sum(six.itervalues(totals))}</td></tr>
          </table>
        </td>
      </tr>
      <tr>
        <td>time_elapsed</td>
        <td>${time}</td>
      </tr>
      <tr>
        <td>name</td>
        <td>${name}</td>
      </tr>
      <tr>
        <td>options</td>
        <td>${options}</td>
      </tr>
      <tr>
        <td>info</td>
        <td>
          <table>
            % for key, sub in sorted(six.iteritems(info)):
              % if isinstance(sub, str):
                <tr><td>${str}</td></tr>
              % else:
                % for subkey, value in sorted(six.iteritems(sub)):
                <tr><td>${subkey}</td><td><pre>${value}</pre></td></tr>
                % endfor
              % endif
            % endfor
          </table>
        </td>
      </tr>
    </table>
    <p>
      <a href="../index.html">Back to summary</a>
    </p>
  </body>
</html>
