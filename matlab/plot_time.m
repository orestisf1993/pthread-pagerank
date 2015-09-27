Folder = '../time_results';
FileName = textscan(ls(Folder),'%s');
FileName = FileName{1}{1};

format long
fid = fopen([Folder '/' FileName]);
% R_data = fscanf(fid,'%d %d %d %f',[4 inf])';
STGE = textscan(fid,'%d %d %d %f');
% Raw_data = textscan(fid,'%c %d %d');
fclose(fid);
% 67 is C in ACSII and 68 is D in Ascii
Size = STGE{1};
Threads = STGE{2};
Generations = STGE{3};
Exec_Time  = STGE{4};


sizeDict = Size(1);
threadDict = Threads(1);
genDict = Generations(1);
for i=1:length(Size)
    
    if not(sizeDict == Size(i))
        sizeDict = [sizeDict Size(i)];
    end
    
    if not(threadDict == Threads(i))
        threadDict = [threadDict Threads(i)];
    end
    
    if not(genDict == Generations(i))
        genDict = [genDict Generations(i)];
    end
    
    
  
end

sizeDict = sort(sizeDict);
threadDict = sort(threadDict);
Time_data = zeros(length(sizeDict),length(threadDict));


for i = 1:length(Size)
    s = find(sizeDict == Size(i));
    t = find(threadDict == Threads(i));
    Time_data(s,t) = Exec_Time(i);
    
end

Gen_data = zeros(length(sizeDict),1);

for i = 1:length(sizeDict)
    g = find(Size == sizeDict(i));
    Gen_data(i) = Generations(g(1));
    
end

figure(1)
plot(Time_data)
legend(num2str((threadDict(:))),'Location','NorthWest')
set(gca, 'Xtick',1:length(sizeDict),'XTickLabel', sizeDict);
xlabel('Number of nodes')
ylabel('time (sec)')

figure(2)
plot(Gen_data)
set(gca, 'Xtick',1:length(sizeDict),'XTickLabel', sizeDict);
xlabel('Number of nodes')
ylabel('Generations til convergeance')
