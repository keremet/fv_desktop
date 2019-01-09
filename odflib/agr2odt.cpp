//Предусмотреть договор без графика - возврат одним платежом
#include <stdio.h>
#include <vector>
#include "num2word.h"
#include "odt_content_editor.h"
#ifdef _WIN32
	#include <windows.h>
	#include <shellapi.h>
#endif

using namespace std;

typedef struct{
	bool pol;
	string info;
} SMem;

typedef struct{
	string date;
	double rate;
} SRate;

void getMemberAgrData(int agr_id, vector<SMem> *v){
	SMem s;
//0 - займодавец	
	s.pol=1;
	s.info="Соколов Андрей Юрьевич, зарегистрированный по адресу г. Киров, ул. Чапаева, д. 5, кв. 71, паспорт серия 3304 № 532894 выдан 31 мая 2005 года УВД Ленинского района г. Кирова";
	v->push_back(s);
//1 - первый созаемщик	
	s.pol=1;
	s.info="Черепов Антон Андреевич, зарегистрированный по адресу Кировская обл, г. Киров, ул. Ульяновская, д.6, кв. 57, паспорт серия 3310 № 055765 выдан 17 февраля 2011 года Отделом УФМС России по Кировской области в Октябрьском районе города Кирова";
	v->push_back(s);
/*	s.pol=1;
	s.info="Кряжев Игорь Александрович, зарегистрированный по адресу Кировская обл., г. Киров, ул. Космонавта Волкова, д.3, кв. 168, паспорт серия 3309 №983419 выдан 29 января 2010 года отделом УФМС России по Кировской области в Ленинском районе города Кирова";
	v->push_back(s);	*/
}

void getRateAgrData(int agr_id, vector<SRate> *vM, vector<SRate> *vP){
	SRate s;
	s.date="13.06.2013";
	s.rate=20;
	vM->push_back(s);
	s.date="13.07.2013";
	s.rate=13;
	vM->push_back(s);
	s.date="13.06.2013";
	s.rate=40;
	vP->push_back(s);
}

double getAgrLimit(int agr_id){
	return 25436;
}

int sched_rows_before;

void addSchedRow(ODTContentEditor *editor){
	editor->insertRow("график", sched_rows_before, "<table:table-row>"
     "<table:table-cell table:style-name=\"ячейка_графика\" office:value-type=\"string\">"
      "<text:p text:style-name=\"ORV_P_CENTER\">Дата</text:p>"
     "</table:table-cell>"
     "<table:table-cell table:style-name=\"ячейка_графика\" office:value-type=\"string\">"
      "<text:p text:style-name=\"ORV_P_END\">Сумма долга</text:p>"
     "</table:table-cell>"
     "<table:table-cell table:style-name=\"ячейка_графика\" office:value-type=\"string\">"
      "<text:p text:style-name=\"ORV_P_END\">Процент</text:p>"
     "</table:table-cell>"
     "<table:table-cell table:style-name=\"ячейка_графика\" office:value-type=\"string\">"
      "<text:p text:style-name=\"ORV_P_END\">Сумма платежа</text:p>"
     "</table:table-cell>"
    "</table:table-row>");
	sched_rows_before++;
}

void fillProcField(ODTContentEditor *editor, vector<SRate> *vr, string fieldName){
	string sProc;
	if(vr->size()==0){
		sProc="!!! НЕ УКАЗАНО !!!";
	}else{
		//DOUBLE2STR (*vr)[0].rate
		sProc="20";
		sProc+="% годовых";		
		if(vr->size()>1){
			sProc+=" с "+(*vr)[0].date;
			
			for(int i=1;i<vr->size();i++){
				sProc+=", ";
				//DOUBLE2STR (*vr)[i].rate
				sProc+="13";
				sProc+="% годовых с "+(*vr)[i].date;
			}
		}
	}
	editor->replaceField(fieldName, sProc.c_str());	
}

int agr2odt(int agr_id){
	sched_rows_before=1;
	double sum=getAgrLimit(agr_id);
	char ssum[500];
	vector<SMem> vSMem;
	getMemberAgrData(agr_id, &vSMem);
	if(vSMem.size()<1+1)
		return 1;
	vector<SRate> vMainRates, vPrRates;
	getRateAgrData(agr_id, &vMainRates, &vPrRates);
	if(vMainRates.size()<1)
		return 2;
    try
    {
		snprintf(ssum, sizeof(ssum),"%i", (int)sum);
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

		

	editor.replaceField("signed", "13.06.2013");
	editor.replaceField("plan_end_date", "30.09.2013");
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
 
	fillProcField(&editor, &vMainRates, "proc");
	if(vPrRates.size()==0)
		editor.replaceFieldWithParagraph("pr_proc", " ");
	else	
		fillProcField(&editor, &vPrRates, "pr_proc");
 
	double proc_sum=0;
	double pl_sum=0;
	addSchedRow(&editor);
	addSchedRow(&editor);
	
	//DOUBLE2STR proc_sum
	editor.replaceField("proc_sum", "1");
	editor.replaceField("pl_sum", "2");
	
	
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
#endif	
	return 0;
}

int main(int argc, char** argv){
	return agr2odt(12);
}
