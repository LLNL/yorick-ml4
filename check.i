plug_dir, _("./", plug_dir());
require,"ml4.i";

FileType="Check file for the yorick ml4 plugin";
d12=dist(12);
dd12=double(d12);
ig1000=indgen(1000);

ml4write,"test.mat",FileType,"FileType","w";
ml4write,"test.mat",d12,"d12","a";
ml4write,"test.mat",dd12,"dd12","a";
ml4write,"test.mat",ig1000,"ig1000","a";

ml4scan,"test.mat";


v=ml4read("test.mat","FileType");
if (v!=FileType) {
  write, format="\"%s\"!=\"%s\"\n", v, FileType; 
  error, "Error reading FileType";
 }

v=ml4read("test.mat","d12");
if (anyof(v!=d12)) {
  v;
  error, "Error reading d12";
 }

v=ml4read("test.mat","dd12");
if (anyof(v!=dd12)) {
  v;
  error, "Error reading dd12";
 }

v=ml4read("test.mat","ig1000");
if (anyof(v!=int(ig1000))){
  v;
  error, "Error reading ig1000";
 }

ml4write,"test.mat","file section 2","another string","a";
ml4scan,"test.mat";
v=ml4read("test.mat","another string");
if (v!="file section 2") error, "Error reading \"another string\"";

write, format="%s\n", "Reading test.mat OK";
 
ml4write,"test2.mat","Check file for the yorick ml4 plugin","FileType","w";
data=gaussdev([3,64,64,50]);
for (i=1;i<=50;i++) ml4write,"test2.mat",data(,,i),"x","a";
ml4scan,"test2.mat";
for (i=1;i<=50;i++) if (anyof(ml4read("test2.mat","x",1)!=data(,,i)))
                        error, "Error reading data(,,"+pr1(i)+")";
ml4close,"test2.mat";
write, format="%s\n", "Reading test2.mat OK";

remove,"test.mat";
remove,"test2.mat";

quit;
