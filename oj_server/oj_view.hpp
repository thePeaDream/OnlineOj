#pragma once
#include <iostream>
#include "oj_model.hpp"
#include <string>
#include <ctemplate/template.h>
namespace ns_view
{
    using namespace std;
    using namespace ns_model;
    //要渲染的网页，所在的路径
    const std::string template_html_path ="./template_html/"; 
    class View
    {
    public:
        View(){}
        ~View(){}
        void AllQuestionHtml(const vector<Question>& questions,string*html)
        {
            //题目编号 题目标题 题目难度
            //1 形成要被渲染的网页路径
            string src_html = template_html_path + "all_question.html";
            //2 形成根数据字典
            ctemplate::TemplateDictionary dic("all_questions");
            //通过循环遍历，会把所有数据循环放在question_list子字典里
            //在html中，可以通过遍历出所有的题目信息
            //{{#question_list}}
            // <tr>
            //     <td>{{number}}</td>
            //     <td>{{title}}</td>
            //     <td>{{star}}</td>
            // </tr>
            // {{/question_list}}
            for(const auto& q:questions)
            {
                ctemplate::TemplateDictionary *sub_dic = dic.AddSectionDictionary("question_list");
                sub_dic->SetValue("number",q.number);
                sub_dic->SetValue("title",q.title);
                sub_dic->SetValue("star",q.star);
            }

            //3 获取待被渲染的html
            ctemplate::Template * tpl = ctemplate::Template::GetTemplate(src_html,ctemplate::DO_NOT_STRIP);
            
            //4 执行渲染
            tpl->Expand(html,&dic);
        }
        void OneQuestionHtml(const Question& q,string* html)
        {
            //1 形成要被渲染的网页路径
            string src_html = template_html_path + "one_question.html";
            //2 形成字典
            ctemplate::TemplateDictionary dic("question");
            dic.SetValue("header",q.header);
            dic.SetValue("star",q.star);
            dic.SetValue("number",q.number);
            dic.SetValue("title",q.title);
            dic.SetValue("desc",q.desc);
            
            //3 获取待被渲染的html
            ctemplate::Template * tpl = ctemplate::Template::GetTemplate(src_html,ctemplate::DO_NOT_STRIP);
            //开始渲染
            tpl->Expand(html,&dic);
        }
    };
}