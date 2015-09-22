/* author 	: zhangqi
 * filename : BSONCreator.cpp
 * version  : 2015/09/15
 */

#include "BSONCreator.h"

using mongo::BSONObjBuilder;
using mongo::BSONElement;  
using mongo::BSONObj;
using mongo::BSONArray;
using mongo::BSONArrayBuilder;


BSONObj createSetUserStockCfg(std::string user,kvpair value,bool exist)
{
	BSONObj retObj;
	int 	index = 0;
	
	BSONObjBuilder fieldbuild,stockbuild,finalbuild,c_bob;
	c_bob.append("UID",user);

	std::string uid,stkcode;
	int bulltin = 0,run = 1;
	float max_price = 0.0f,min_price = 0.0f;
	long long int increment = 0;
	
	for(; index<value.keyary.size(); index++)
	{
		char k[1024] = {0};
		if(!exist)
			sprintf(k,"%s",value.keyary[index].c_str());
		else
			sprintf(k,"stocks.$.%s",value.keyary[index].c_str());
		
		std::string key = value.keyary[index];
		if(key == "uid")
		{
			uid = value.valueary[index];
			if(!exist)
				stockbuild.append(value.keyary[index].c_str(),uid.c_str());
		}
		else if(key == "stockcode")
		{
			stkcode = value.valueary[index];
			stockbuild.append(k,stkcode.c_str());
		}
		else if(key == "max_price")
		{
			sscanf(value.valueary[index].c_str(),"%f",&max_price);
			if(max_price != 0.0f)
				stockbuild.append(k,max_price);
		}
		else if(key == "min_price")
		{
			sscanf(value.valueary[index].c_str(),"%f",&min_price);
			if(min_price != 0.0f)
				stockbuild.append(k,min_price);
		}
		else if(key == "bulltin")
		{
			sscanf(value.valueary[index].c_str(),"%d",&bulltin);
			stockbuild.append(k,bulltin);
		}
		else if(key == "run")
		{
			sscanf(value.valueary[index].c_str(),"%f",&max_price);
			if(max_price != 0.0f)
				stockbuild.append(k,max_price);
		}
		else if(key == "incrementId")
		{
			sscanf(value.valueary[index].c_str(),"%ld",&increment);
			stockbuild.append(k,increment);
		}
	}
	if(!exist)
	{
		fieldbuild.append("stocks",stockbuild.obj());
		finalbuild.append("$push",fieldbuild.obj());
	}
	else
	{
		c_bob.append("stocks.stockcode",stkcode.c_str());
		finalbuild.append("$set",stockbuild.obj());
	}
	//mongo::Query query = c_bob.obj();
	return finalbuild.obj();	
}

BSONObj createCommonSub(std::string dbcoll,std::string kv,std::string mkey,std::string arrkey,kvpair value,bool exist)
{
	std::string subkey;
	int pos = arrkey.find(".");
	BSONObjBuilder fieldbuild,subbuild,finalbuild;
	if(pos > 0)
		subkey = arrkey.substr(pos+1,arrkey.size()-pos-1);
	else
		return finalbuild.obj();
	
	int index = 0;
		
	if(!exist)
	{
		for(; index < value.valueary.size();index++)
		{
			subbuild.append(value.keyary[index].c_str(),value.valueary[index].c_str());
		}
		fieldbuild.append(subkey,subbuild.obj());
		finalbuild.append("$push",fieldbuild.obj());
	}
	else
	{
		for(index=1;index<value.valueary.size();index++)
		{
			char k[1024]={0};
			sprintf(k,"%s.$.%s",subkey.c_str(),value.keyary[index].c_str());
			subbuild.append(k,value.valueary[index].c_str());
		}
		finalbuild.append("$set",subbuild.obj());
	}
	return finalbuild.obj();
}
/*

typedef struct _tag_apptype_info
{
	unsigned int 	apptype;
	int	flag;
	std::string  	lastuid;
	long long int 	lasttime;
	long long int 	firsttime;
	std::string 	appversion;
}apptypeinfo;

typedef struct _tag_mobile_apptypes
{
	std::string token;
	std::vector<apptypeinfo>	apptypes;
	long long int incrementID;
	std::string osversion;
	std::string comment;
}mobile_apptypes;
*/
void parseMobileToken(BSONObj obj,mobile_apptypes& app)
{
	app.token = obj.getStringField("Token");
	if(obj.getField("incrementID").ok())
		app.incrementID = obj.getField("incrementID").Long();
	else
		app.incrementID = -1;
	app.osversion = obj.getStringField("osversion");
	app.comment = obj.getStringField("comment");

	std::vector<BSONElement> elements = obj["apptypes"].Array();
	std::vector<BSONElement>::iterator it = elements.begin();

	for(; it != elements.end(); ++it)
	{
		BSONObj appobj = it->Obj();
		apptypeinfo appinfo;
		appinfo.apptype = appobj.getIntField("apptype");		
		if(appobj.getField("flag").ok())
			appinfo.flag = appobj.getIntField("flag");
		else 
			appinfo.flag = 0;
		appinfo.lastuid = appobj.getStringField("lastuid");
		if(appobj.getField("lasttime").ok())
			appinfo.lasttime = appobj.getField("lasttime").Long();
		else
			appinfo.lasttime = 0;
		appinfo.appversion = appobj.getStringField("appversion");
		app.apptypes.push_back(appinfo);
	}
}
