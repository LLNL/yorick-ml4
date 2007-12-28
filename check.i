require,"ml4.i";
ml4write,"test.mat","Check file for the yorick ml4 plugin","FileType","w";
ml4write,"test.mat",dist(12),"d12","a";
ml4write,"test.mat",indgen(1000),"ig100","a";
ml4scan,"test.mat";
v=ml4read("test.mat","ig100");
info,v;
window;
plot,v;
pause,500;
tv,ml4read("test.mat","d12");
v=ml4read("test.mat","FileType");
write,format="And btw, the title was: \n%s\n",v;
ml4write,"test.mat","file section 2","another string","a";
ml4scan,"test.mat";

write,format="%s\n","You can also append variable with the same name (is that smart?):";
ml4write,"test2.mat","Check file for the yorick ml4 plugin","FileType","w";
for (i=1;i<=50;i++) ml4write,"test2.mat",gaussdev([2,64,64]),"x","a";
write,format="%s\n","and read them, leaving the file opened between reads";
animate,1;
for (i=1;i<=50;i++) tv,ml4read("test2.mat","x",1);
animate,0;
ml4close,"test2.mat";

write,format="%s\n","same file in Little endian (note that matlabio only reads LE)";
ml4write,"test3.mat","Check file for the yorick ml4 plugin","FileType","w",endian='L';
ml4write,"test3.mat","little","endian","a",endian='L';
ml4write,"test3.mat",dist(12),"d12","a",endian='L';
ml4write,"test3.mat",indgen(1000),"ig100","a",endian='L';
ml4scan,"test3.mat";

remove,"test.mat";
remove,"test2.mat";
remove,"test3.mat";
