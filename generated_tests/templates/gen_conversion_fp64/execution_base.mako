<%! from six.moves import range %>\
[require]
${require}\
${next.body()}\
[test]
% for conversion in conversions:
uniform ${uniform_from_type} from \
% for i in range(amount):
${conversion['from']} \
% endfor

uniform ${uniform_to_type} to \
% for i in range(amount):
${conversion['to']} \
% endfor

draw rect -1 -1 2 2
probe all rgba 0.0 1.0 0.0 1.0

% endfor
