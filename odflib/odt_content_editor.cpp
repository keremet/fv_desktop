#include <errno.h>
#include "odt_content_editor.h"
#include <string.h>

using namespace std;

void ODTContentEditor::replaceAll(string *str, const string key, const string value){
	for(string::size_type pos=0;(pos = str->find(key, pos))!=string::npos;){
		str->replace(pos,key.length(), value);
	}
}

void ODTContentEditor::replaceAll(const string key, const string value){
	replaceAll(&content, key, value);
}

void ODTContentEditor::replaceField(const string key, const string value){
	//~ fprintf(stderr,"|%s|%s|\n", key.c_str(), value.c_str());
	/*string field = "<text:user-field-get text:name=\""+key+"\">"+key+"</text:user-field-get>";	
	replaceAll(content, field, value); */
	string fldBegin = "<text:user-field-get text:name=\""+key+"\">";
	for(string::size_type pos=0;(pos = content.find(fldBegin, pos))!=string::npos;){
		string::size_type fldEndPos = content.find("</text:user-field-get>", pos);
		content.replace(pos,fldEndPos+22 /*strlen("</text:user-field-get>")*/ - pos, value);
		pos += value.length();
	}		
}


void ODTContentEditor::replaceFieldWithParagraph(const string key, const string value){
	//fprintf(stderr,"|%s|%s|\n", key.c_str(), value.c_str());
	string field = "<text:user-field-get text:name=\""+key+"\">"+key+"</text:user-field-get>";	
	for(string::size_type pos=0;(pos = content.find(field, pos))!=string::npos;){
		string::size_type paraEndPos = content.find("</text:p>", pos)+9; //strlen("</text:p>");
		string::size_type paraBeginPos = content.rfind("<text:p ", pos); //strlen("</text:p>");
		content.replace(paraBeginPos,paraEndPos-paraBeginPos, value);
		pos = paraBeginPos+value.length();
	}
}

void ODTContentEditor::addStyles(const string value){
	string::size_type stylesPos = content.find("</office:automatic-styles>");
	if(stylesPos!=string::npos)
		content.insert(stylesPos,value);
}



void ODTContentEditor::addRow(const string key, const string value){
	//printf("%s\n", key.c_str());
	string::size_type tablePos = content.find("<table:table table:name=\"" +key+ "\"");
	if(tablePos!=string::npos){
//		printf("%i\n", tablePos);
		string::size_type tableEndPos = content.find("</table:table>", tablePos);
//		printf("%i\n", tableEndPos);
		if(tableEndPos!=string::npos)
			content.insert(tableEndPos,value);
	}
}

void ODTContentEditor::insertRow(const string key, unsigned rowsBefore, string value){
	//printf("%s\n", key.c_str());
	string::size_type tablePos = content.find("<table:table table:name=\"" +key+ "\"");
	if(tablePos!=string::npos){
		string::size_type tableEndPos = content.find("</table:table>", tablePos);
		if(tableEndPos!=string::npos){
			string::size_type rowPos = content.find("<table:table-row", tablePos);
			if(rowPos==string::npos) return;
			for(;rowsBefore;rowsBefore--){
				rowPos = content.find("</table:table-row>", rowPos+1);
				if(rowPos==string::npos) return;
				rowPos += 18 /*strlen("</table:table-row>")*/ ;
			}
			//printf("%i\n\n%s", rowsBefore, value.c_str());
			if(rowPos>tableEndPos) return;
			content.insert(rowPos,value);
		}
	}
}

void ODTContentEditor::deleteTable(const string tableName){	
	string::size_type tablePos = content.find("<table:table table:name=\"" +tableName+ "\"");
	if(tablePos!=string::npos){
		string::size_type tableEndPos = content.find("</table:table>", tablePos);
		if(tableEndPos!=string::npos){
			content.erase(tablePos, tableEndPos+14 /*strlen("</table:table>")*/ - tablePos);
		}
	}
}

string ODTContentEditor::getContent(const char* odtFileName, bool main_file){
	int i, err;
	char errstr[1024];	
	struct zip *zs;
	int idx;
    if ((zs=zip_open(odtFileName, 0, &err)) == NULL) {
		zip_error_to_str(errstr, sizeof(errstr), err, errno);
		fprintf(stderr, "cannot open  %s: %s\n",odtFileName,errstr);
		return "";
    }	
    if ((idx=zip_name_locate(zs, "content.xml", ZIP_FL_NODIR|ZIP_FL_NOCASE)) != -1) {
		struct zip_file *zf;
	//Чтение содержимого		
		string content="";
		if ((zf=zip_fopen_index(zs, idx, 0)) == NULL) {
			fprintf(stderr, "cannot open file %i in archive: %s\n",idx,zip_strerror(zs));
			return "";
		}
		int n;
		char buf[8192];
		while ((n=zip_fread(zf, buf, sizeof(buf))) > 0) {
			//~ for(int i=0;i<n;i++)
				//~ cout << buf[i];
			for(int i=0;i<n;i++)
				content+=buf[i];
		}		
		if (n < 0) {
			fprintf(stderr, "error reading file %i in archive: %s\n",idx,zip_file_strerror(zf));
			zip_fclose(zf);
			return "";
		}
		if(main_file){
			m_zs = zs;
			m_idx = idx;
		}else
			zip_fclose(zf);
		return content;
	}else{
		fprintf(stderr, "zip_name_locate error\n");
	}	
	return "";
}

#ifdef	ODT_CONTENT_EDITOR_H_USE_COPY

void ODTContentEditor::renameStylesAndObjects(string *str, const string prefix){	
	for(string::size_type stylePos = 0;(stylePos = str->find("<style:style style:name=\"", stylePos))!=string::npos;stylePos+=26){
		string::size_type styleNamePos=stylePos+25;//strlen("<style:style style:name=\"");
		string styleName=str->substr(styleNamePos, str->find('"',styleNamePos)-styleNamePos);
		replaceAll(str, string("name=\"")+styleName+'"', string("name=\"")+prefix+'_'+styleName+'"');
	}
}

void ODTContentEditor::addOfficeText(const string value){
	content.insert(content.find("</office:text>"),value);
}

string ODTContentEditor::getTextBetween(const std::string *str, const char* begin, const char* end){
	string::size_type  posB = str->find(begin);
	if(posB==string::npos) return "";
	posB = str->find('>',posB);
	if(posB==string::npos) return "";
	posB++;
	string::size_type  posE=str->find(end,posB); 
	if(posE==string::npos) return "";
	return str->substr(posB,posE-posB); 
}

string ODTContentEditor::getOfficeText(const std::string *str){	
	return getTextBetween(str, "<office:text", "</office:text>");
}

string ODTContentEditor::getStylesText(const std::string *str){
	return getTextBetween(str, "<office:automatic-styles", "</office:automatic-styles>");
}

void ODTContentEditor::copyfile(const char* filename){
	string odtFileContent = getContent(filename, false); 
	if(odtFileContent.empty()){
		fprintf(stderr, "error at getContent %s\n", filename);
		return;
	}
	const char* pc_slash = strrchr(filename, SLASH);
	string prefix;
	if(pc_slash)
		prefix = pc_slash+1;
	else
		prefix = filename;
	renameStylesAndObjects(&odtFileContent, prefix);		
	//printf("%s\n", odtFileContent.c_str());
	string external_office_text = getOfficeText(&odtFileContent);
	if(external_office_text.empty()){
		fprintf(stderr, "error at getOfficeText %s\n", filename);
		return;		
	}
	string external_styles_text = getStylesText(&odtFileContent);
	if(external_styles_text.empty()){
		fprintf(stderr, "error at getStylesText %s\n", filename);
		return;		
	}
	addOfficeText(external_office_text);
	addStyles(external_styles_text);	
}

void ODTContentEditor::copy(){
	addOfficeText(office_text);
}
#endif



ODTContentEditor::ODTContentEditor(const char* inputfilename, const char* _outputfilename
#ifdef	ODT_CONTENT_EDITOR_H_USE_COPY
	, bool usecopy
#endif
){
	content = getContent(inputfilename,true);
#ifdef	ODT_CONTENT_EDITOR_H_USE_COPY
	if(usecopy)
		office_text=getOfficeText(&content);
#endif		
	outputfilename=_outputfilename;
}

ODTContentEditor::~ODTContentEditor(){
	struct zip_source *source=zip_source_buffer(m_zs, content.c_str(), content.length(), 0);		
	if(source==NULL){
		fprintf(stderr, "error at zip_source_buffer\n");
		return;
	}
	char errstr[1024];
	int r, err;
	if((r = zip_replace(m_zs,m_idx,source))<0){
		zip_error_to_str(errstr, sizeof(errstr), err, errno);			
		fprintf(stderr, "cannot replace %i %s\n",r,errstr);
		return;
	}
	if((r = zip_close_into_new_file(m_zs, outputfilename.c_str()))<0)
		fprintf(stderr, "zip_close error %i %s\n", r, outputfilename.c_str());
}

/*
void refreshTextFields(string *content, const string value){
	list<string> textFieldsList;
	for(string::size_type pos=0;(pos = content->find("<text:bookmark-start text:name=\"", pos))!=string::npos;){
		pos+=32; //strlen("<text:bookmark-start text:name=\"")
		string name= content->substr(pos , content->find("\"/>", pos)-pos);
		textFieldsList.push_back (name);
	}
	for (list<string>::iterator it = textFieldsList.begin(); it != textFieldsList.end(); it++){
		fprintf(stderr,"FieldNameL=%s\n", it->c_str());
		string::size_type bookmarkPos = content->find("<text:bookmark-start text:name=\"" + *it +"\"/>"); 
		if(bookmarkPos==string::npos){
			fprintf(stderr,"BookMark=%s not found\n", it->c_str());
			continue;
		}		
		string::size_type listRootPos = content->rfind("<text:list xml:id=\"", bookmarkPos);
		if(listRootPos==string::npos){
			fprintf(stderr,"List root for BookMark=%s not found\n", it->c_str());
			continue;
		}
		ostringstream oss;
		for(string::size_type pos=listRootPos;(pos = content->find("<text:list>", pos+1))!=string::npos;){
			oss<<'.';
		}
		fprintf(stderr,"value=%s\n", oss.str().c_str());
		//replaceAll(content, const string key, oss.str())
	}
}
*/
