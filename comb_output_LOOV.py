# -*- coding: utf-8 -*-

###########################################################################
# Copyright (C) <2014>  <Johann POIGNANT>
#
# This file is part of LOOV.
#
# LOOV is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# LOOV is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with LOOV.  If not, see <http://www.gnu.org/licenses/>.
###########################################################################

import glob
import os
import operator
import unicodedata
import Levenshtein
import sys
import getopt
import fnmatch
from multiprocessing import Pool

def usage():
    print "usage : "
    print "    -i --pathin    : path where files are saves from OCR"
    print "    -o --pathout   : output path for combination"
    print "    -t --pathtemp  : temp path, (must be empty or not exist)"
    print "    -s --SRILM     : path to SRILM, can be download at http://www-speech.sri.com/projects/srilm/download.html"
    print "    -r --recursive : find recursively *.txt"
    print "    -n --normalize : normalize = 0 : without normalize , normalize = 1 : without case and accent, normalize = 2 : without accent, default 1"


def cleanString(s, normalize): 
    s=s.replace("`","'")    
    s=s.replace("\n","")
    
    list_c="abcdefghijklmnopqrstuvwxyz0123456789àâäçéèêëîïôöùûüœæ%-><'\".:\\t\\n"    
    list_c2="ABCDEFGHIJKLMNOPQRSTUVWXYZÀÂÄÇÉÈÊËÎÏÔÖÙÛÜÆŒ|!\\t\\n"
          
    if normalize==1 or normalize == 2:
        list_c="abcdefghijklmnopqrstuvwxyz0123456789%->'\".:"    
        list_c2="ABCDEFGHIJKLMNOPQRSTUVWXYZ|!"  
        s=s.replace('Æ','AE') 
        s=s.replace('Œ','OE')
        s=s.replace('œ','oe')
        s=s.replace('æ','ae')
        
    dic_upper={'l':'I'}
    dic_lower={'I':'l'}
    new_s=''
    for i in range(len(s)):
        if s[i] in list_c or s[i] in list_c2:
            if i!=0 and  i!=len(s)-1:
                if s[i] in dic_upper and s[i+1] in list_c2 and s[i-1] in list_c2:
                    new_s+=dic_upper[s[i]]
                elif s[i] in dic_lower and s[i+1] in list_c and s[i-1] in list_c:
                    new_s+=dic_lower[s[i]]
                else:
                    new_s+=s[i]    
            else:
                new_s+=s[i]  
        else:
            new_s+=s[i]
    s=new_s 
    if s == '':
        return s
    
                 
    dic_upper={'|':'I','!':'I'}
    dic_lower={'|':'l','!':'l'}
    new_s=''
    for i in range(len(s)):
        if s[i] in list_c or s[i] in list_c2:
            if i==0 :
                if s[i] in dic_upper and s[i+1] in list_c2:
                    new_s+=dic_upper[s[i]]
                elif s[i] in dic_lower and s[i+1] in list_c:
                    new_s+=dic_lower[s[i]]
                else:
                    new_s+=s[i]  
            elif i==len(s)-1:
                if s[i] in dic_upper and s[i-1] in list_c2:
                    new_s+=dic_upper[s[i]] 
                elif s[i] in dic_lower and s[i-1] in list_c:
                    new_s+=dic_lower[s[i]]                     
                else:
                    new_s+=s[i]
            else:
                if s[i] in dic_upper and (s[i+1] in list_c2 or s[i-1] in list_c2):
                    new_s+=dic_upper[s[i]]  
                elif s[i] in dic_lower and (s[i+1] in list_c or s[i-1] in list_c):
                    new_s+=dic_lower[s[i]]                    
                else:
                    new_s+=s[i]
        else:
            new_s+=s[i]       
    s=new_s 
    if s == '':
        return s  
        
    if isinstance(s,str):
        s = unicode(s,"utf8","replace")
    if normalize==1 or normalize==2:
        s=unicodedata.normalize('NFD',s)
        s=s.encode('ascii','ignore') 
    else:
        s=s.encode('ISO-8859-1','replace')

    if normalize==1:
        s=s.lower()
    if normalize==1 or normalize==2:
        for c in s:
            if c not in list_c:
                s=s.replace(c," ")

    if s == '':
        return s  
    while '  ' in s:
        s = s.replace('  ', ' ')
        if s == '':
            return s 
    if s == '':
        return s                   
    while s[0]==' ':
        s = s[1:]
        if s == '':
            return s  
    if s == '':
        return s                  
    while s[:-2]==' \n':
        s = s[:-2]+'\n'
        if s == '':
            return s   
    if s == '':
        return s                 
    s=s.replace(" \t","")  
    s=s.replace("\t","")       
                    
    
    new_s=''
    for i in range(len(s)-1):
        if s[i]=='q' and s[i+1]!='u' and s[i+1]!=' ' :
            new_s+='o'
        elif s[i]=='Q' and s[i+1]!='U' and s[i+1]!=' ' :
            new_s+='O'
        else:
            new_s+=s[i]
    new_s+=s[-1]
    s=new_s  
      
    
    return s
            
def post_tr_had_oc(str, car_echange):
    str=str.replace('II','M')
    str=str.replace('ii','n')

    list_nb="0123456789"
    previous_c=' '
    current_c=' '
    next_c=' '
    count_c=0
    for c in str: 
        previous_c=current_c
        current_c=next_c
        next_c=c       
        if current_c =='8' and previous_c not in list_nb and next_c not in list_nb:
            str=str.replace('8','S')
        count_c+=1
    if next_c=='8' and current_c not in list_nb:
        str=str.replace('8','S')
    return str   

def combination(arg):
    file = arg[0]
    pathout = arg[1]
    pathtemp = arg[2]
    SRILM = arg[3]
    normalize = arg[4]
    count_box=arg[5]

    print file
    (file_filepath,file_filename) = os.path.split(file)
    (file_shortname,file_extension) = os.path.splitext(file_filename)        
    f = open(file,'r')
            
    if pathout != '':
        f_out=open(pathout+"/"+file_shortname+'_uni'+file_extension,'w')
    else:
        f_out=open(file_filepath+"/"+file_shortname+'_uni'+file_extension,'w')
    
    if pathtemp == '':
        pathtemp=file_filepath+'/temp'+str(count_box)                            
    if not os.path.exists(pathtemp):
        os.popen('mkdir '+pathtemp)        
    files_temp=glob.glob(pathtemp+'/*')
    if files_temp!=[]:
        print ('temp path not empty')
        sys.exit(2)         
            
    f_out.write("#start_frame\tstop_frame\tymin\txmin\tymax\txmax\ttext_comb\tconfidence_value\ttext_average\n")
    text={}
    text2=[]
    text2_temp=[]
    text_avg=[]
    start_box=False
    start_fr=0
    stop_fr=0
    ymin=0
    xmin=0
    ymax=0
    xmax=0
    count_l1=0
    for line in f.readlines():
        #line = line.replace('\n','')
        l = line.split("\t")  
        if "end_box\n" == l[0] and start_box==True and len(l)==1:                
            if len(text2_temp)!=0 and len(text2_temp)%len(text_avg)==0:         
                for nb_l in range(len(text_avg)):
                    count_l=0
                    for t in text2_temp:
                        if count_l==nb_l:
                            text2.append(t)
                        count_l+=1
                        if count_l==len(text_avg):
                            count_l=0

                    
                    f_temp=open(pathtemp+'/temp.txt','w')
                    t_temp=text_avg[nb_l-1].replace(" ","_")         
                    f_temp.write("1 1 "+str(len(t_temp))+" ")   
                    for c in t_temp:
                        f_temp.write(c+" ")
                    f_temp.write("\n")
                    
                    for t in text2:
                        t_temp=t.replace(" ","_")         
                        f_temp.write("1 1 "+str(len(t_temp))+" ")   
                        for c in t_temp:
                            f_temp.write(c+" ")
                        f_temp.write("\n")
                    f_temp.close()
                    
                    os.popen(SRILM+"/nbest-lattice -use-mesh -nbest "+pathtemp+"/temp.txt -write "+pathtemp+"/line1.mesh")
                    os.popen(SRILM+"/lattice-tool -in-lattice "+pathtemp+"/line1.mesh -read-mesh -viterbi-decode | sed 's/<s>//g' | sed 's/<\/s>//g' | sed 's/.*\.mesh//g' >> "+pathtemp+"/combined.post.nolm")
                    
                    f_temp=open(pathtemp+"/combined.post.nolm","r")
                    combined=f_temp.read()
                    f_temp.close()
                    combined=combined.replace(" ","").replace("_"," ").replace("\n","")
                    os.popen("rm -R "+pathtemp+"/*")

                    comb=Levenshtein.ratio(combined,text_avg[nb_l-1])
                    nb_comb=1.0
                    for t in text2:
                        comb+=Levenshtein.ratio(combined,t)
                        nb_comb+=1
                    comb/=nb_comb

                    while combined[-1:]==' ':
                        combined=combined[:-1]
                    words=combined.replace('      ',' ').replace('     ',' ').replace('    ',' ').replace('   ',' ').replace('  ',' ').split(" ")

                    len_average_word=0.0
                    nb_w=0.0
                    for word in words :
                        len_average_word+=float(len(word))
                        nb_w+=1.0
                    avg_w=float(len_average_word)/float(nb_w)
                    if (avg_w>=2.0 and comb>0.7) or avg_w>=3.0:                       
                        f_out.write(str(start_fr)+"\t"+str(stop_fr)+"\t"+ymin+"\t"+xmin+"\t"+ymax+"\t"+xmax+"\t"+combined+"\t"+str(comb)+"\t"+text_avg[nb_l-1]+"\n")
                    text2=[]
            start_box=False
            text2_temp=[]
            text_avg=[]
            
        if start_box==True and "start_box" != l[0] and "end_box\n" != l[0]:
            if len(l)==8:
                pos_text=7                
            if len(l)==7:
                pos_text=6  
            if int(l[0])>=start_fr and int(l[1])<=stop_fr and len(l)==pos_text+1 :
                text_temp = cleanString(post_tr_had_oc(l[pos_text], " "), normalize)
                if text_temp!="":
                    words=text_temp.split(" ")
                    len_average_word=0.0
                    for word in words :
                        len_average_word+=len(word)
                    len_average_word/=len(words)                        
                    text2_temp.append(text_temp) 
            count_l1+=0
            if count_l1==len(text_avg)-1:
                count_l=0                    

        if "start_box" == l[0] and start_box==False:
            start_box=True 
            
            if len(l)==9:
                pos_text=8
            if len(l)==8:
                pos_text=7

            text_avg.append(cleanString(post_tr_had_oc(l[pos_text]," "), normalize)) 
            count_l=1
            ymin=l[3]
            xmin=l[4]
            ymax=l[5]
            xmax=l[6] 
            start_fr = int(l[1])
            stop_fr = int(l[2])  
    f.close()
    f_out.close()
    os.popen('rm -rf '+pathtemp)


if __name__ == '__main__':    
    codage='utf-8' 
    pos_text=0
    pathin = ""
    pathout = ""  
    pathtemp = ""
    SRILM = ""
    recursive=False
    normalize=1
    core=1    
    try:                                
        opts, args = getopt.getopt(sys.argv[1:], "hi:o:t:s:rn:e:", ["help", "pathin=", "pathout=", "pathtemp=", "SRILM=", "recursive", 'normalize=', "core="]) 
    except getopt.GetoptError:           
        usage()                          
        sys.exit(2) 
    for opt, arg in opts:                
        if opt in ("-h", "--help"):      
            usage()                     
            sys.exit()               
        elif opt in ("-i", "--pathin"): 
            pathin = arg     
        elif opt in ("-o", "--pathout"): 
            pathout = arg    
        elif opt in ("-t", "--pathtemp"): 
            pathtemp = arg         
        elif opt in ("-s", "--SRILM"): 
            SRILM = arg 
        elif opt in ("-r", "--recursive"): 
            recursive = True   
        elif opt in ("-n", "--normalize"): 
            normalize= int(arg)
        elif opt in ("-e", "-core"): 
            core = int(arg)                         
    if pathin =='':
        print 'enter input path files'
        sys.exit(2)          
    if SRILM=='':
        print 'enter path for SRILM'
        sys.exit(2) 

    if os.path.isfile(pathin):
        files=[pathin]
    else : 
        if recursive:
            files = []
            for root, dirnames, filenames in os.walk(pathin+'/'):
                for filename in fnmatch.filter(filenames, '*.txt'):
                    files.append(os.path.join(root, filename))
        else:
            files  = glob.glob(pathin+'/*.txt')

    p = Pool(core)
    tab_arg=[]
    count_file=0
    for file in sorted(files):
        r=tab_arg.append([file, pathout, pathtemp, SRILM, normalize, count_file])
        count_file+=1
    p.map(combination, tab_arg)

