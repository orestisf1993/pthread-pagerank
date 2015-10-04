function [] = portrait_print(h, filename)
set(h, 'PaperType', 'a4','PaperOrientation', 'portrait')
h.PaperUnits = 'centimeters';
h.PaperPosition = [0 0 21 29.7];
h.PaperPositionMode = 'manual';
print(filename,'-dpdf','-r0')
end

