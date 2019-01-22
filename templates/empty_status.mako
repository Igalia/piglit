<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="UTF-8">
    <title>Result summary</title>
    <link rel="stylesheet" href="status.css">
  </head>
  <body>
    <h1>Result summary</h1>
    <p>Currently showing: ${page}</p>
    <p>Show:
      ## Index is a logical choice to put first, it will always be a link
      ## and we don't want in preceded by a |
      <a href="index.html">index</a>
      % for i in pages:
        % if i == page:
          | ${i}
        % else:
          | <a href="${i}.html">${i}</a>
        % endif
      % endfor
    </p>
    <h1>No ${page}</h1>
  </body>
</html>
