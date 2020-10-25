#include <stdio.h>
#include <regex.h>
#include <memory.h>
#include <stdlib.h>
#include <string.h>

#define true 1
#define false 0

// ==============================================================================================
// Common tools
// ==============================================================================================

// 去除字符串右边指定字符
void rstrip(char *pt_str, char ch){
	if((pt_str != NULL) && (*pt_str != '\0')){
		char *pt_tmp = pt_str + strlen(pt_str) - 1;
		while(*pt_tmp == ch){
			*pt_tmp = '\0';
			pt_tmp--;
		}
	}
}

// 判断字符串中是否包含某字符
int str_contains_char(char *pt_str, char ch){
	for(int i = 0; i <= strlen(pt_str); i++){
		if(pt_str[i] == ch)
			return true;
	}
	return false;
}

// 打印一行传入的字符
void print_char_line(char ch, int length){
	for(int i = 0; i < length; i++)
		printf("%c", ch);
	printf("\n");
}

struct RegResults{
	char os[1000];
	char dev[1000];
	char ver[1000];
};

void initialize_reg_results(struct RegResults* reg_results){
	strcpy(reg_results->os, "");
	strcpy(reg_results->dev, "");
	strcpy(reg_results->ver, "");
}

void print_reg_results(struct RegResults* reg_results){
	printf("os: %s\n", reg_results->os);
	printf("dev: %s\n", reg_results->dev);
	printf("ver: %s\n", reg_results->ver);
}

// void str_to_lower(char* pt_str)
// {
// 	char* cur_ch = pt_str;
// 	while(*cur_ch != '\0'){
// 		if((*cur_ch >= 'A') && (*cur_ch <= 'Z'))
// 			*cur_ch = *cur_ch + 32; // *cur_ch = *cur_ch + ('a' - 'A');
// 		cur_ch++;
// 	}
// }

char* match_on_match(char *sub_str_be_matched, char *sub_pattern){
	char errbuf[1024];
	char *match_res;
	match_res = (char *)malloc(1000); 
	regex_t reg;
	int err = 10;
	int nm = 1;
	regmatch_t pmatch[nm];

	if(regcomp(&reg, sub_pattern, REG_EXTENDED|REG_ICASE) < 0){
		regerror(err, &reg, errbuf, sizeof(errbuf));
		printf("[sub_match] sub_str_be_matched: \"%s\"\n", sub_str_be_matched);
		printf("[sub_match] sub_pattern: \"%s\"\n", sub_pattern);
		printf("[sub_match] err: %s\n", errbuf);
	}

	err = regexec(&reg, sub_str_be_matched, nm, pmatch, 0);

	if(err == REG_NOMATCH){
		// printf("[sub_match] sub_str_be_matched: \"%s\"\n", sub_str_be_matched);
		// printf("[sub_match] sub_pattern: \"%s\"\n", sub_pattern);
		// printf("[sub_match] no match\n");
		exit(-1);
	}else if(err){
		regerror(err,&reg,errbuf,sizeof(errbuf));
		printf("[sub_match] sub_str_be_matched: \"%s\"\n", sub_str_be_matched);
		printf("[sub_match] sub_pattern: \"%s\"\n", sub_pattern);
		printf("[sub_match] err: %s\n",errbuf);
		exit(-1);
	}

	for(int i = 0; i < nm && pmatch[i].rm_so != -1; i++){
		int len = pmatch[i].rm_eo - pmatch[i].rm_so;
		if(len){
			memset(match_res, '\0', sizeof(match_res));
			memcpy(match_res, sub_str_be_matched + pmatch[i].rm_so, len);
			// printf("[sub_match] sub_str_be_matched: \"%s\"\n", sub_str_be_matched);
			// printf("[sub_match] sub_pattern: \"%s\"\n", sub_pattern);
			// printf("[sub_match] match_res: \"%s\"\n", match_res);
		}
	}
	return match_res;
}

void match_on_match_for_once(struct RegResults* reg_results, char *match, char *sub_pattern, int patterns_mode){
	char *sub_match_res;
	sub_match_res = match_on_match(match, sub_pattern);
	if(patterns_mode == 1) strcpy(reg_results->os, sub_match_res);
	if(patterns_mode == 2) strcpy(reg_results->dev, sub_match_res);
	if(patterns_mode == 3) strcpy(reg_results->ver, sub_match_res);
	free(sub_match_res); //一定要进行free，否则会造成内存泄露
}

void match_on_match_for_twice(struct RegResults* reg_results, char *match, char *sub_pattern_r1, char *sub_pattern_r2, int patterns_mode){
	char *sub_match_res_r1;
	char *sub_match_res_r2;
	sub_match_res_r1 = match_on_match(match, sub_pattern_r1);
	sub_match_res_r2 = match_on_match(sub_match_res_r1, sub_pattern_r2);
	if(patterns_mode == 1) strcpy(reg_results->os, sub_match_res_r2);
	if(patterns_mode == 2) strcpy(reg_results->dev, sub_match_res_r2);
	if(patterns_mode == 3) strcpy(reg_results->ver, sub_match_res_r2);
	free(sub_match_res_r1); //一定要进行free，否则会造成内存泄露
	free(sub_match_res_r2); //一定要进行free，否则会造成内存泄露
}

// ==============================================================================================
// Andoird
// ==============================================================================================

void update_android_reg_results(struct RegResults* android_reg_results, int pattern_i, char *match){
	if(pattern_i == 0)
		strcpy(android_reg_results->dev, match);
	if(pattern_i == 1)
		strcpy(android_reg_results->os, match);
	if(pattern_i == 2){
		strcpy(android_reg_results->dev, match);
		rstrip(android_reg_results->dev, ' ');
	}
	if((pattern_i == 3) || (pattern_i == 4)){
		if(!str_contains_char(match, '/'))
			strcpy(android_reg_results->ver, match);
	}
}

void android_feature_extration(struct RegResults* android_reg_results, char *str_be_matched){
	char patterns[][1000] = {
		"model/([a-zA-Z0-9 -]*) ",
		"Android ([0-9.]*); ",
		"([a-zA-Z]{2}-[a-zA-Z]{2}; )?([a-zA-Z0-9 -]*[^)(Build)(MIUI))/-])*", // 空格必须在-之前，否则会报段错误
		"Build/([a-zA-Z0-9.-]*)?",
		"MIUI/([a-zA-Z0-9.-]*)+"
	};
	int pattern_cnt = sizeof(patterns) / sizeof(patterns[0]); // number of patterns

	regex_t reg;
	int offset = 0;
	int offset_add_flag;
	int offset_add_len;
	initialize_reg_results(android_reg_results);
	for(int pattern_i = 0; pattern_i < pattern_cnt; pattern_i++){
		// printf("----> [Android] Processing pattern %i/%i\n", pattern_i + 1, pattern_cnt);
		char *pattern = patterns[pattern_i];
		// printf("----> [Android] Pattern to be used is \"%s\"\n", pattern);
		// printf("----> [Android] The present string to be matched is \"%s\"\n", str_be_matched + offset);
		char errbuf[1024];
		char match[1000];
		int err = 100;
		int nm = 10;
		regmatch_t pmatch[nm];

		if(regcomp(&reg, pattern, REG_EXTENDED|REG_ICASE) < 0){ // REG_ICASE表示匹配字母时忽略大小写
			regerror(err, &reg, errbuf, sizeof(errbuf));
			printf("err: %s\n", errbuf);
	 	}

	 	err = regexec(&reg, str_be_matched + offset, nm, pmatch, 0);

		if(err == REG_NOMATCH){
			// printf("no match\n");
			if(pattern_i == 1)
			{
				// printf("================> It's not Android.\n");
				return; // 安卓的如果连第一个os版本号都匹配不到那就说明肯定不是安卓的了，可以直接return，省得浪费时间匹配后面的
			}
			// exit(-1);
			continue;
		}
		else if(err){
			regerror(err, &reg, errbuf, sizeof(errbuf));
			printf("err: %s\n", errbuf);
			// exit(-1);
			continue;
		}

		offset_add_flag = false;
		for(int i = 0; i < nm && pmatch[i].rm_so != -1; i++){
			int len = pmatch[i].rm_eo - pmatch[i].rm_so;
			if(len){
				memset(match, '\0', sizeof(match));
				memcpy(match, str_be_matched + offset + pmatch[i].rm_so, len);
				// printf(">> [%i] match: \"%s\"\n", i, match);
				if(!offset_add_flag)
				{
					offset_add_len = pmatch[i].rm_so + len;
					offset_add_flag = true;
				}
				update_android_reg_results(android_reg_results, pattern_i, match);
			}
		}
		if(offset_add_flag)
			offset += offset_add_len;
		regfree(&reg);
	}
}

// ==============================================================================================
// iOS
// ==============================================================================================

void update_ios_reg_results_for_osver(struct RegResults* ios_reg_results, int pattern_i, char *match){
	if(pattern_i == 0){
		char *sub_pattern = "[0-9._]+";
		match_on_match_for_once(ios_reg_results, match, sub_pattern, 1);
	}
	if(pattern_i == 1){
		char *sub_pattern_r1 = "[/;]{1}( )?[0-9._]+";
		char *sub_pattern_r2 = "[0-9._]+";
		match_on_match_for_twice(ios_reg_results, match, sub_pattern_r1, sub_pattern_r2, 1);
	}
}

int update_ios_reg_results_for_dev(struct RegResults* ios_reg_results, int pattern_i, char *match){
	if(pattern_i == 0){
		char *sub_str = "OS";
		if(!strstr(match, sub_str)){
			strcpy(ios_reg_results->dev, match);
		}
		else{
			return true; // 表示匹配错了，误把os字段匹配成dev字段（因为太难实现不匹配“OS”这样的字符串字段的正则表达式，所以只能先匹配后判断是否有“OS”）
		}
	}
	if(pattern_i == 1){
		// char *sub_pattern = "((i?mac)|(iphone)|(ipad)|(ipod)|(i?watch)){1}([a-zA-Z0-9 ]*)(,[0-9]+)+";
		char *sub_pattern = "(,[0-9]+)+";
		match_on_match_for_once(ios_reg_results, match, sub_pattern, 2);
	}
	return false;
}

void update_ios_reg_results_for_devver(struct RegResults* ios_reg_results, int pattern_i, char *match){
	if((pattern_i == 0) || (pattern_i == 1)){
		char *sub_pattern = "[0-9]+[a-zA-Z]+[0-9]+[a-zA-Z]?";
		match_on_match_for_once(ios_reg_results, match, sub_pattern, 3);
	}
	if(pattern_i == 2){
		char *sub_pattern_r1 = ",([0-9]+[a-zA-Z]+[0-9]+[a-zA-Z]?),";
		char *sub_pattern_r2 = "[0-9]+[a-zA-Z]+[0-9]+[a-zA-Z]?";
		match_on_match_for_twice(ios_reg_results, match, sub_pattern_r1, sub_pattern_r2, 3);
	}
}

int ios_regex_matching(struct RegResults* ios_reg_results, char *str_be_matched, char patterns[][1000], int pattern_cnt, int patterns_mode){
	regex_t reg;
	for(int pattern_i = 0; pattern_i < pattern_cnt; pattern_i++){
		if((patterns_mode == 1) && (strlen(ios_reg_results->os) > 0)) continue; // 匹配os的阶段如果ios_reg_results里面已经存有os也就是曾经匹配中了，那就直接continue不用继续匹配了，下面类似。
		if((patterns_mode == 2) && (strlen(ios_reg_results->dev) > 0)) continue;
		if((patterns_mode == 3) && (strlen(ios_reg_results->ver) > 0)) continue;

		// printf("----> [iOS] Processing pattern %i/%i\n", pattern_i + 1, pattern_cnt);
		char *pattern = patterns[pattern_i];
		// printf("----> [iOS] Pattern to be used is \"%s\"\n", pattern);
		// printf("----> [iOS] The present string to be matched is \"%s\"\n", str_be_matched);
		char errbuf[1024];
		char match[1000];
		int err = 100;
		int nm = 1; // 情况太复杂了，统一在匹配出来的长串上进行二次匹配吧
		regmatch_t pmatch[nm];

		if(regcomp(&reg, pattern, REG_EXTENDED|REG_ICASE) < 0){ // REG_ICASE表示匹配字母时忽略大小写
			regerror(err, &reg, errbuf, sizeof(errbuf));
			printf("err: %s\n", errbuf);
	 	}

	 	err = regexec(&reg, str_be_matched, nm, pmatch, 0);

		if(err == REG_NOMATCH){
			// printf("no match\n");
			// exit(-1);
			continue;
		}
		else if(err){
			regerror(err, &reg, errbuf, sizeof(errbuf));
			printf("err: %s\n", errbuf);
			// exit(-1);
			continue;
		}

		for(int i = 0; i < nm && pmatch[i].rm_so != -1; i++){
			int len = pmatch[i].rm_eo - pmatch[i].rm_so;
			if(len){
				memset(match, '\0', sizeof(match));
				memcpy(match, str_be_matched + pmatch[i].rm_so, len);
				// printf(">> [%i] match: \"%s\"\n", i, match);
				// 这里patterns_mode == 1, 2 或者3分别对应Python代码的pattern1，pattern2和pattern3
				if(patterns_mode == 1){
					if((pmatch[i].rm_so == 0) || ((*(str_be_matched + pmatch[i].rm_so - 1) != '/') && (*(str_be_matched + pmatch[i].rm_so - 1) != '_'))){
						update_ios_reg_results_for_osver(ios_reg_results, pattern_i, match);
					}
				}
				else if(patterns_mode == 2){
					int error_match_os;
					error_match_os = update_ios_reg_results_for_dev(ios_reg_results, pattern_i, match);
					if(error_match_os){
						return pmatch[i].rm_eo; // 把os字段的末尾的index返回
					}
				}
				else if(patterns_mode == 3){
					update_ios_reg_results_for_devver(ios_reg_results, pattern_i, match);
				}
			}
		}
		regfree(&reg);
	}
	return false;
}

void ios_regex_matching_for_osver(struct RegResults* ios_reg_results, char *str_be_matched){
	// printf("========> Matching os using ios_regex_matching_for_osver\n");
	char patterns[][1000] = {
		"((i?OS)|(MAC OS X)){1}[ ;,/]?((V)|(Ver)|(Version))?[ ;,/]?([0-9._]+)", // os 1
		"((((i?mac)|(iphone)|(ipad)|(ipod)|(i?watch)){1}([a-zA-Z0-9 ]*)(,[0-9]+)+)|(Mac OS X)){1}[/;]{1}( )?([0-9._]+)", // os 2
	};
	int pattern_cnt = sizeof(patterns) / sizeof(patterns[0]); // number of patterns
	ios_regex_matching(ios_reg_results, str_be_matched, patterns, pattern_cnt, 1);
}

int ios_regex_matching_for_dev(struct RegResults* ios_reg_results, char *str_be_matched){
	// printf("========> Matching dev using ios_regex_matching_for_dev\n");
	char patterns[][1000] = {
		"((i?mac)|(iphone)|(ipad)|(ipod)|(i?watch)){1}([a-zA-Z0-9 ]*)(,[0-9]+)+", // dev 1
		// ",((i?mac)|(iphone)|(ipad)|(ipod)|(i?watch)){1}([a-zA-Z0-9 ]*)(,[0-9]+)+]", // dev 2
	};
	int pattern_cnt = sizeof(patterns) / sizeof(patterns[0]); // number of patterns
	int error_match_os;
	error_match_os = ios_regex_matching(ios_reg_results, str_be_matched, patterns, pattern_cnt, 2);
	return error_match_os;
}

void ios_regex_matching_for_devver(struct RegResults* ios_reg_results, char *str_be_matched){
	// printf("========> Matching ver using ios_regex_matching_for_devver\n");
	char patterns[][1000] = {
		"((build)|(model)|(mobile)){1}[ /]{1}([0-9]+[a-zA-Z]+[0-9]+[a-zA-Z]?)", // ver 1
		"[(;]{1}[ ]?([0-9]+[a-zA-Z]+[0-9]+[a-zA-Z]?)[);]{1}[ ]?", // ver 2
		"((i?OS)|(MAC OS X)){1}[ ;,/]?((V)|(Ver)|(Version))?[ ;,/]?([0-9._]+),([0-9]+[a-zA-Z]+[0-9]+[a-zA-Z]?),",
	};
	int pattern_cnt = sizeof(patterns) / sizeof(patterns[0]); // number of patterns
	ios_regex_matching(ios_reg_results, str_be_matched, patterns, pattern_cnt, 3);
}

void ios_feature_extration(struct RegResults* ios_reg_results, char *str_be_matched){
	initialize_reg_results(ios_reg_results);
	ios_regex_matching_for_osver(ios_reg_results, str_be_matched); // 匹配osver
	int error_match_os;
	error_match_os = ios_regex_matching_for_dev(ios_reg_results, str_be_matched); // 匹配dev
	if(error_match_os){
		// printf("The osver is miss matched as the dev, and it is end with index of %i, so cut it out and match it again!\n", error_match_os);
		ios_regex_matching_for_dev(ios_reg_results, str_be_matched + error_match_os);
	}
	ios_regex_matching_for_devver(ios_reg_results, str_be_matched); // 匹配devver
}

int main(){
	char uals[][5000] = {
		"Dalvik/2.1.0 (Linux; U; Android 5.1; OPPO R9m Build/LMY47I)",
		"Mozilla/5.0 (Linux; U; Android 6.0; zh-cn; HUAWEI NXT-AL10 Build/HUAWEINXT-AL10) AppleWebKit/537.36 (KHTML, like Gecko)Version/4.0 Chrome/37.0.0.0 MQQBrowser/6.9 Mobil???",
		"Mozilla/5.0 (Linux; Android 10; YAL-AL00 Build/HUAWEIYAL-AL00; wv) AppleWebKit/537.36 (KHTML, like Gecko) Version/4.0 Chrome/80.0.3987.99 Mobile Safari/537.36",
		"Dalvik/2.1.0 (Linux; U; Android 10; PCCM00 Build/QKQ1.191021.002",
		"android Mozilla/5.0 Linux; Android 9; PCCM00 Build/PKQ1.190223.001; wv) AppleWebKit/537.36 (KHTML, like Gecko) Version/4.0 Chrome/70.0.3538.110 Mobile Safari/537.36",
		"Dalvik/2.1.0 (Linux; U; Android 7.1.2; Redmi 5 Plus MIUI/V9.6.1.0.NEGCNFD)",
		"Dalvik/2.1.0 (Linux; U; Android 9; MI 9 MIUI/V10.2.35.0.PFACNXM",
		"Dalvik/2.1.0 (Linux; U; Android 9; MI 9 MIUI/",
		"Mozilla/5.0 (Linux; Android 11; Pixel 2 XL) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/84.0.4147.125 Mobile Safari/537.36",
		"Mozilla/5.0 (Linux; U; Android 10; zh-cn; Pixel 3a Build/QQ3A.200805.001) AppleWebKit/537.36 (KHTML, like Gecko)Version/4.0 Chrome/37.0.0.0 MQQBrowser/6.9 Mobile Safari???",
		"Mozilla/5.0 BiliDroid/6.6.0 (bbcallen@gmail.com) os/android model/Mi 10 mobi_app/android build/6060600 channel/xiaomi innerVer/6060610 osVer/10 network/2",
		"Mozilla/5.0 (Linux; Android 9; LYA-L09; Build/HUAWEILYA-L09; wv) AppleWebKit/537.36 (KHTML, like Gecko) Version/4.0 Chrome/84.0.4147.125 Mobile Safari/537.36"
	
		"QrvIVideo/11.8.1(ios;com.iqiyi.player;iOS10.15.6;MacBookAir9,1) Corejar",
		"QrYIVideo/9.19.5(ios;com.pps.test;ios13.6.1;iphone12,1) Corejar",
		"QiviVideo_Uni/9.19.5(iPhone;iOS 13.6.1; Scale/2.00) QYSDWebImage",
		"Qivipadvideo/11.8.0(iPad; iOS 13.6.1; Scale/2.00）QYSDWebImage/",
		"com.iqiyi.player/5.19.3(Mac OS X Version 10.15.6 (Build 19G2021)",

		"Alipay/10.2.0.6020 iOS/13.6.1",

		"Mozi1a/5.0 (Macintosh; Intel Mac OS X 10_15_6) AppleWebKit/605.1.15 (KHTML, like Gecko)",
		"Mozilla/5.0 (Macintosh; Intel Mac OS X 10_16) AppleWebKit/605.1.15 (KHTML, like Gecko)",
		"Mozilla/5.0 (iPhone; CPU iPhone OS 13_6_1 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) Mobile/15E148",

		"server-bag [Mac OS X,10.15.6,19G2021,iMac19,2]",
		"server-bag [iPhone OS,14.0,18A5351d,iPad7,5]",
		"server-bag [Watch OS,6.0,17R575,Watch5,1]",
		"server-bag [macOS,11.0,20A5354i,MacBookPro15,2]",
		"server-bag [Mac OS X,10.15.6,19G2021,Macmini8,1]",
		"server-bag [Mac OS X,10.15.6,19G2021,MacBookAir9,1]",
		"server-bag [iPhone OS, 13.6.1,17G80,iPhone12,8]",

		"itunesstored/1.0 iOS/13.6.1 model/iPad11,1 hwp/t8020 build/17G80 (5; dt:209)",

		"server-bag [iPhone OS,13.6.1,17G80,iPad8,1]",
		"com.apple.invitation-registration [iPhone OS,13.6.1,17G80,iPad8,1]",
		"QIYIVideo/11.5.0 (iOS;com.qiyi.hd;iOS13.6.1;iPad8,1)",
		"server-bag [iPhone OS,13.6.1,17G80,iPad11,1]",
		"Dalvik/2.1.0 (Linux; U; Android 9; LYA-L09 Build/HUAWEILYA-L09)",
		"Mozilla/5.0 (iPhone; CPU iPhone OS 13_6_1 like Mac OS X)",
		"QIYIVideo/11.8.1 (iOS;com.iqiyi.player;iOS10.15.6;MacBookAir9,1) Corejar",
		"QIYIVideo/10.8.5 (iOS;com.qiyi.iphone;iOS14.0;iPhone10,6) Corejar",
		"爱奇艺 10.8.5 rv:20190911182000 (iPhone; iOS 14.0; zh-Hans_US)",
		"gamed/5.10.19.4.8.16.5.4.2 (iPhone7,1; 10.3.3; 114G60; GameKit-471.4.113)",
		"Mac OS X/10.13.6 (17G65)",
		"iPad5,3/11.2.5 (15D60)",
		"Mac OS X/10.11.6 (15G19009)",
		"iPhone10,3/12.3.1 (16F203)",
		"iPhone10,1/12.4 (16G77)",
		"iPhone8,2/12.0 (16A366)",
		"iphone x,4/13.0(hnds)",
		"iphone max,4/5.(hnds)",
		"itunesstored/1.0 iOS/13.6.1 model/iPhone12,1 hwp/t8020 build/17G80 (6; dt:203)",
		"aiqiyi/ios/13.2.1" // 自己造的一个特殊情况的数据
	};	// 待搜索的字符串
	int data_cnt = sizeof(uals) / sizeof(uals[0]); //数据的行数

	for(int data_i = 0; data_i < data_cnt; data_i++){
		print_char_line('=', 150);
		printf("================> Processing string %i/%i\n", data_i + 1, data_cnt);
		char *str_be_matched = uals[data_i];
		printf("================> The whole string to be matched is \"%s\"\n", str_be_matched);
		
		// 先用安卓的模式匹配一下看是否是安卓的
		struct RegResults android_reg_results; //声明一个android_reg_results结构体变量用于储存匹配结果（以引用形式传递）
		android_feature_extration(&android_reg_results, str_be_matched);
		if((strlen(android_reg_results.os) > 0) || (strlen(android_reg_results.dev) > 0) || (strlen(android_reg_results.ver) > 0)){ // 说明是安卓的，打印下结果
			printf("================> It's Android, and the regex results are: \n");
			print_reg_results(&android_reg_results);
		}
		else{ // 说明不是安卓的，所以用iOS的模式匹配一下
			struct RegResults ios_reg_results; //声明一个ios_reg_results结构体变量用于储存匹配结果（以引用形式传递）
			ios_feature_extration(&ios_reg_results, str_be_matched);
			if((strlen(ios_reg_results.os) > 0) || (strlen(ios_reg_results.dev) > 0) || (strlen(ios_reg_results.ver) > 0)){
				printf("================> It's iOS, and the regex results are: \n");
				print_reg_results(&ios_reg_results);
			}
			else{
				printf("================> It's neither Android nor iOS.\n");
			}
		}
	}
	return 0;
}
