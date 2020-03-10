//Предусмотреть договор без графика - возврат одним платежом
#include <stdio.h>
#include <vector>
#include "agr2odt.h"
#include "odflib/num2word.h"
#include "odflib/odt_content_editor.h"
#include "julianday.h"
#include "financial.h"
#ifdef _WIN32
	#include <windows.h>
	#include <shellapi.h>
#else
	#include <unistd.h>
#endif

using namespace std;

typedef struct{
	string date;
	double rate;
} SRate;

typedef struct{
	vector<SRate> vMainRates;
	vector<SRate> vPrRates;
} SRates;

typedef struct{
	ODTContentEditor *editor;
	financial proc_sum;
	financial pl_sum;
	int sched_rows_before;
} SStateContext;

static void fillProcField(ODTContentEditor *editor, vector<SRate> *vr, string fieldName){
	string sProc;
	if(vr->size()==0){
		sProc="!!! НЕ УКАЗАНО !!!";
	}else{
		char str_rate[20];
		snprintf(str_rate, sizeof(str_rate),"%g", (*vr)[0].rate);
		sProc=str_rate;
		sProc+="% годовых";		
		if(vr->size()>1){
			sProc+=" с "+(*vr)[0].date;
			
			for(int i=1;i<vr->size();i++){
				sProc+=", ";
				snprintf(str_rate, sizeof(str_rate),"%g", (*vr)[i].rate);
				sProc+=str_rate;
				sProc+="% годовых с "+(*vr)[i].date;
			}
		}
	}
	editor->replaceField(fieldName, sProc.c_str());	
}

static int cb_fv_rate(void* param, PARAMS_ID_fv_rate, PARAMS_fv_rate){
	SRates *rates = (SRates*)param;
	SRate r;
	r.rate = value;
	char str_date[100];
	JD2full_str(id, str_date, sizeof(str_date));
	r.date = str_date;
	if (type == 1)
		rates->vMainRates.push_back(r);
	else if (type == 2)
		rates->vPrRates.push_back(r);
}

static int cb_fv_schedule_state(void* param, int id, PARAMS_fv_schedule_state){ 
	SStateContext *sc = (SStateContext *)param;	
	char str_state_date[2+1+2+1+4 + 1];
	JD2str(state_date, str_state_date);	
	sc->editor->insertRow("график", sc->sched_rows_before, 
		string ("<table:table-row>"
		 "<table:table-cell table:style-name=\"ячейка_графика\" office:value-type=\"string\">"
		  "<text:p text:style-name=\"ORV_P_CENTER\">")+str_state_date+"</text:p>"
		 "</table:table-cell>"
		 "<table:table-cell table:style-name=\"ячейка_графика\" office:value-type=\"string\">"
		  "<text:p text:style-name=\"ORV_P_END\">"+financial2str(remainder)+"</text:p>"
		 "</table:table-cell>"
		 "<table:table-cell table:style-name=\"ячейка_графика\" office:value-type=\"string\">"
		  "<text:p text:style-name=\"ORV_P_END\">"+financial2str(interest)+"</text:p>"
		 "</table:table-cell>"
		 "<table:table-cell table:style-name=\"ячейка_графика\" office:value-type=\"string\">"
		  "<text:p text:style-name=\"ORV_P_END\">"+financial2str(payment)+"</text:p>"
		 "</table:table-cell>"
		"</table:table-row>"
	);
	sc->sched_rows_before++;		
	sc->proc_sum+=interest;
	sc->pl_sum+=payment;
	
	return 0;
}


int agr2odt(int agr_id, DB* db){
	vector<SMem> vSMem;
	db->getMemberAgrData(agr_id, &vSMem);
	if(vSMem.size()<1+1)
		return 1;
	
	SRates rates;
	db->select_fv_rate(agr_id, cb_fv_rate, &rates);
	if(rates.vMainRates.size()<1)
		return 2;
		
	financial sum=db->getAgrSum(agr_id)/100;
	char ssum[500];
	try
    {
		snprintf(ssum, sizeof(ssum),"%i", sum);
		std::string s = string_functions::number_in_words(ssum);
		snprintf(ssum, sizeof(ssum),"%i (%s)", (int)sum, s.c_str());
    }
    catch (std::exception& e)
    {
		printf("%s %s\n", e.what(), ssum);
		return 3;
    }	
	

	char output_file_name[255];
	time_t rawtime = time (NULL);
	struct tm * timeinfo = localtime ( &rawtime );	
	snprintf(output_file_name, sizeof(output_file_name), "agr_%i___%i_%i_%i__%i_%i_%i.odt", agr_id, timeinfo->tm_year+1900,timeinfo->tm_mon+1,	timeinfo->tm_mday,timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);	
{
	ODTContentEditor editor("agr_template.odt", output_file_name);
	editor.addStyles("<style:style style:name=\"ORV_P1\" style:family=\"paragraph\" style:parent-style-name=\"Standard\">"
   "<style:paragraph-properties fo:text-align=\"justify\" style:justify-single-word=\"false\"/>"
   "<style:text-properties fo:font-size=\"10pt\" style:text-underline-style=\"none\"/>"
  "</style:style>"
  "<style:style style:name=\"ячейка_графика\" style:family=\"table-cell\">"
   "<style:table-cell-properties fo:padding=\"0.097cm\" fo:border=\"0.002cm solid #000000\"/>"
  "</style:style>"
  "<style:style style:name=\"ORV_P_CENTER\" style:family=\"paragraph\" style:parent-style-name=\"Table_20_Contents\">"
   "<style:paragraph-properties fo:text-align=\"center\" style:justify-single-word=\"false\"/>"
   "<style:text-properties style:font-name=\"Times New Roman\" fo:font-size=\"10pt\" style:font-name-complex=\"Times New Roman\"/>"
  "</style:style>"
  "<style:style style:name=\"ORV_P_END\" style:family=\"paragraph\" style:parent-style-name=\"Table_20_Contents\">"
   "<style:paragraph-properties fo:text-align=\"end\" style:justify-single-word=\"false\"/>"
   "<style:text-properties style:font-name=\"Times New Roman\" fo:font-size=\"10pt\" style:font-name-complex=\"Times New Roman\"/>"
  "</style:style>"  
  );
	int date;
	char str_date[100];
	date = db->getSigned(agr_id);
	JD2full_str(date, str_date, sizeof(str_date));	
	editor.replaceField("signed", str_date);
	
	date = db->getPlanEndDate(agr_id);	
	JD2full_str(date, str_date, sizeof(str_date));		
	editor.replaceField("plan_end_date", str_date);
	
	editor.replaceField("sum", ssum);
	editor.replaceField("zaimodav", vSMem[0].info);
	editor.replaceField("zDavChisPril", (( vSMem[0].pol )?"ый":"ая"));	
 {	
	string zaemschiki_podp;
	string dop_podp;
	for(int i=1;i<vSMem.size();i++){
		zaemschiki_podp+="<text:p text:style-name=\"ORV_P1\">"+vSMem[i].info+", </text:p>"
				+"<text:p text:style-name=\"ORV_P1\"> ___________________/________________/ </text:p>"
				"<text:p text:style-name=\"ORV_P1\"> </text:p>";
		if(i>=2)
			dop_podp+="<text:p text:style-name=\"ORV_P1\"> ___________________/________________/ </text:p>"
				"<text:p text:style-name=\"ORV_P1\"> </text:p>";
	}
	editor.replaceFieldWithParagraph("zaemschiki_podp", zaemschiki_podp);
	editor.replaceFieldWithParagraph("dop_podp", dop_podp);
 }	
 
	fillProcField(&editor, &(rates.vMainRates), "proc");
	if(rates.vPrRates.size()==0)
		editor.replaceFieldWithParagraph("pr_proc", " ");
	else	
		fillProcField(&editor, &(rates.vPrRates), "pr_proc");
{ 
	
	SStateContext sc;
	sc.proc_sum=0;
	sc.pl_sum=0;
	sc.sched_rows_before=1;
	sc.editor=&editor;
	int last_sched_id = db->getLastScheduleID(agr_id);
	db->select_fv_schedule_state(last_sched_id, cb_fv_schedule_state, &sc);
	editor.replaceField("proc_sum", financial2str(sc.proc_sum).c_str());
	editor.replaceField("pl_sum", financial2str(sc.pl_sum).c_str());
}	
	
	if(vSMem.size()==1+1){
		editor.replaceField("zChis", "");
		editor.replaceField("zChisDP", "у");
		editor.replaceField("zChisTP", "ом");
		editor.replaceField("zChisSpjazh", "е");
		editor.replaceField("zChisPril", ((vSMem[1].pol)?"ый":"ая"));
		editor.replaceFieldWithParagraph("zChisIsMnozh", " ");
		editor.replaceFieldWithParagraph("dop_podp", " ");
		editor.replaceField("zaemschiki", vSMem[1].info+",");
	}else{
		editor.replaceField("zChis", "и");
		editor.replaceField("zChisDP", "ам");
		editor.replaceField("zChisTP", "ами");
		editor.replaceField("zChisSpjazh", "ю");
		editor.replaceField("zChisPril", "ые");
		editor.replaceField("zChisIsMnozh", " ");
		string zaemschiki;
		for(int i=1;i<vSMem.size();i++)
			zaemschiki+="<text:p text:style-name=\"ORV_P1\">"+vSMem[i].info+", </text:p>";
		editor.replaceFieldWithParagraph("zaemschiki", zaemschiki);
	}
}
#ifdef _WIN32
	ShellExecute(NULL, "open", output_file_name, NULL, NULL, SW_SHOWNORMAL);
#else	
	pid_t pID = vfork();
	if (pID == 0){                // child  
		execl("/usr/bin/soffice","/usr/bin/soffice", output_file_name, 0);
	}else if (pID < 0){            // failed to fork
		fprintf(stderr,"failed to fork\n");
		return -1;
	}
#endif	
	return 0;
}

//~ int main(int argc, char** argv){
	//~ return agr2odt(12);
//~ }
