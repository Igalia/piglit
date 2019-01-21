<?xml version="1.0" encoding="UTF-8"?>

<%!
  import posixpath  # this must be posixpath, since we want /'s not \'s
  import re

  from six.moves import range


  def feat_result(result):
      """Percentage result string"""
      return '{}/{}'.format(result[0], result[1])


  def escape_filename(key):
      """Avoid reserved characters in filenames."""
      return re.sub(r'[<>:"|?*#]', '_', key)


  def escape_pathname(key):
      """ Remove / and \\ from names """
      return re.sub(r'[/\\]', '_', key)


  def normalize_href(href):
      """Force backward slashes in URLs."""
      return href.replace('\\', '/')
%>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
 "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
  <head>
    <meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
    <title>Result summary</title>
    <link rel="stylesheet" href="index.css" type="text/css" />
  </head>
  <body>
    <h1>Feature readiness</h1>
    <table>
      <colgroup>
        ## Name Column
        <col />

        ## Status columns
        ## Create an additional column for each summary
        % for _ in range(len(results.results)):
        <col />
        % endfor
      </colgroup>
      <tr>
        <th/>
        % for res in results.results:
          <th class="head"><b>${res.name}</b><br />\
          (<a href="${normalize_href(posixpath.join(escape_pathname(res.name), 'index.html'))}">info</a>)</th>
        % endfor
      </tr>
      % for feature in results.features:
        <tr>
        ## Add the left most column, the feature name
        <td>
          <div class="group">
            <b>${feature}</b>
          </div>
        </td>
        ## add the feature totals
        % for res in results.results:
          <td class="${results.feat_status[res.name][feature]}">
            <b>${feat_result(results.feat_fractions[res.name][feature])}</b>
          </td>
        % endfor
        </tr>
      % endfor
    </table>
  </body>
</html>
