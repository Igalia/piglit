<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
 "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
  <head>
    <meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
    <title>Result summary</title>
    <link rel="stylesheet" href="index.css" type="text/css" />
  </head>
  <body>
    <h1>Result summary</h1>
    <p>Currently showing: ${page}</p>
    <p>Show:
      % if page == 'all':
        all
      % else:
        <a href="index.html">all</a>
      % endif
      % for i in pages:
        % if i == page:
          | ${i}
        % else:
          | <a href="${i}.html">${i}</a>
        % endif
      % endfor
    </p>
    <table>
      <colgroup>
        ## Name Column
        <col />

        ## Status columns
        ## Create an additional column for each summary
        % for _ in xrange(colnum):
        <col />
        % endfor
      </colgroup>
      % for line in results:
        % if line['type'] == "newRow":
        <tr>
        % elif line['type'] == "endRow":
        </tr>
        % elif line['type'] == "groupRow":
          <td>
            <div class="${line['class']}" style="margin-left: ${line['indent']}em">
              <b>${line['text']}</b>
            </div>
          </td>
        % elif line['type'] == "testRow":
          <td>
            <div class="${line['class']}" style="margin-left: ${line['indent']}em">
              ${line['text']}
            </div>
          </td>
        % elif line['type'] == "groupResult":
          <td class="${line['class']}">
            <b>${line['text']}</b>
          </td>
        % elif line['type'] == "testResult":
          <td class="${line['class']}">
          ## If the result is in the excluded results page list from
          ## argparse, just print the text, otherwise add the link
          % if line['class'] not in exclude:
            <a href="${line['href']}">
              ${line['text']}
            </a>
          % else:
            ${line['text']}
          % endif
          </td>
        % elif line['type'] == "subtestResult":
          <td class="${line['class']}">
            ${line['text']}
          </td>
        % elif line['type'] == "other":
          ${line['text']}
        % endif
      % endfor
    </table>
  </body>
</html>
