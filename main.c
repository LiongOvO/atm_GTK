/*----------ATM自动取款机----------*/
//Author:LiongZHANG HaiyangZHANG YouzengGao XiaoweiWang

/*----------ATM自动取款机----------*/

/*----------头文件----------*/

#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "rfid_lib.h"
#include "sqlite3.h"
#define UART_DEV  	"/dev/ttySAC2"
/*----------全局变量----------*/
char kahao_char[30];
GtkWidget* window_main;
GtkWidget* window_fuwu;
GtkWidget* window_zhuanzhang;
GtkWidget* window_yue;
GtkWidget* window_tuika;
GtkWidget* window_gaimi;
GtkWidget* window_cunkuan;
GtkWidget* window_qukuan;
GtkWidget* window_qita;
GtkWidget* label_yue;
GtkWidget* label_yues;
GtkWidget* label_uyues;
GtkWidget* label_kahaos;
GtkWidget* label_cunkuan_yue;
GtkWidget* label_qukuan_yue;
GtkWidget* label_zhuanzhang_yue;
GtkWidget* label_card_num_num;
GtkWidget* label_card_num_qukuan;
GtkWidget* label_card_num_gaimi;
GtkWidget* label_card_num_zhuanzhang;
GtkWidget* entry_passwd;
GtkWidget* entry_card_num;
GtkWidget* entry_card_num2;
GtkWidget* entry_card_zhuanzhang;
GtkWidget* entry_yuanmima;
GtkWidget* entry_xinmima;
GtkWidget* entry_querenxinmima;
GtkWidget* entry_cunkuanjine;
GtkWidget* entry_qukuanjine;
/*----------刷卡按钮回调函数----------*/
void shuaka_function(void)
{

	int len,i;char type;
	//len:读取id的长度   type：表示你的卡类型
	unsigned char id[18] = {0};
	//id就是卡号
	//捕获信号
	uart_rfid_init(UART_DEV);//设备初始化
	char hehe[30] = "";
	while(1)
	{
		if(len = get_rfid_card_id(id,&type)){
			// printf("%c类卡卡号:",type);
			//------------打印卡号----------------
			// for(i=0;i<len;i++)
			// 	printf("%02x ",id[i]);			
			for (i = 0; i < len; ++i)
			{
				printf("0x%02x", id[i]);
				
			}
			if (len == 4)
			{
				sprintf(hehe, "%02x%02x%02x%02x", id[0],id[1],id[2],id[3]);
				strcpy(kahao_char, hehe);
				gtk_entry_set_text(GTK_ENTRY(entry_card_num),kahao_char);
				// printf("---%s===\n", hehe);
			}
			else
			{
				sprintf(hehe, "%02x %02x %02x %02x %02x %02x %02x %02x", id[0],id[1],id[2],id[3], id[4],id[5],id[6],id[7]);
				strcpy(kahao_char, hehe);
				gtk_entry_set_text(GTK_ENTRY(entry_card_num),kahao_char);
				// printf("---%s===\n", hehe);
			}
			break;
		}
	}
}

/*----------图片背景----------*/
void chang_background(GtkWidget *widget, int w, int h, const gchar *path)
{
	gtk_widget_set_app_paintable(widget, TRUE);		//允许窗口可以绘图
	gtk_widget_realize(widget);	
	gtk_widget_queue_draw(widget);
	GdkPixbuf *src_pixbuf = gdk_pixbuf_new_from_file(path, NULL);	// 创建图片资源对象
	GdkPixbuf *dst_pixbuf = gdk_pixbuf_scale_simple(src_pixbuf, w, h, GDK_INTERP_BILINEAR);
	GdkPixmap *pixmap = NULL;
	gdk_pixbuf_render_pixmap_and_mask(dst_pixbuf, &pixmap, NULL, 128);
	gdk_window_set_back_pixmap(widget->window, pixmap, FALSE);
	// 释放资源
	g_object_unref(src_pixbuf);
	g_object_unref(dst_pixbuf);
	g_object_unref(pixmap);
}
/*----------字体设置大小----------*/
static void set_widget_font_size(GtkWidget *widget, int size, gboolean is_button)  
{  
    GtkWidget *labelChild;    
    PangoFontDescription *font;    
    gint fontSize = size;    
    font = pango_font_description_from_string("Sans");          //"Sans"字体名     
    pango_font_description_set_size(font, fontSize*PANGO_SCALE);//设置字体大小     
    if(is_button){  
        labelChild = gtk_bin_get_child(GTK_BIN(widget));//取出GtkButton里的label    
    }else{  
        labelChild = widget;  
    }  
    //设置label的字体，这样这个GtkButton上面显示的字体就变了  
    gtk_widget_modify_font(GTK_WIDGET(labelChild), font);  
    pango_font_description_free(font);  
}
/*----------数据清空----------*/
void login_clean(void)
{
	gtk_entry_set_text(GTK_ENTRY(entry_passwd),"");
	gtk_entry_set_text(GTK_ENTRY(entry_card_num),"");
}
void zhuanzhang_clean(void)
{
	gtk_entry_set_text(GTK_ENTRY(entry_card_zhuanzhang),"");
	gtk_entry_set_text(GTK_ENTRY(entry_card_num2),"");
}
void gaimi_clean(void)
{
	gtk_entry_set_text(GTK_ENTRY(entry_yuanmima),"");
	gtk_entry_set_text(GTK_ENTRY(entry_xinmima),"");
	gtk_entry_set_text(GTK_ENTRY(entry_querenxinmima),"");
}
void cunkuan_clean(void)
{
	gtk_entry_set_text(GTK_ENTRY(entry_cunkuanjine),"");
}
void qukuan_clean(void)
{
	gtk_entry_set_text(GTK_ENTRY(entry_qukuanjine),"");
}
/*----------登录按钮回调函数----------*/
void button_login_function(GtkButton* button,gpointer user_data)
{
	const char * id= gtk_entry_get_text(GTK_ENTRY(entry_card_num));
	strcpy(kahao_char,id);

	const char * passwd = gtk_entry_get_text(GTK_ENTRY(entry_passwd));
	sqlite3* db;
	sqlite3_open("bugbank.db",&db);
	char sq_denglu[100] = "";
	sprintf(sq_denglu,"select *from bank where card_num = \'%s\'and passwd = \'%s\';",id,passwd);
	char **table = NULL;
	int r =0,c =0;
	char *errmsg = NULL;
	sqlite3_get_table(db,sq_denglu,&table,&r,&c,&errmsg);
	if(r!=0)
	{
		gtk_widget_show_all(window_fuwu);
		gtk_widget_hide_all(window_main);
		gtk_entry_set_text(GTK_ENTRY(entry_passwd),"");
		gtk_entry_set_text(GTK_ENTRY(entry_card_num),"");
		sqlite3_close(db);
		return;
	}
	else 
		gtk_entry_set_text(GTK_ENTRY(entry_card_num),"卡号或密码错误，请重新输入");
		gtk_entry_set_text(GTK_ENTRY(entry_passwd),"");
		sqlite3_close(db);
}
void button_fanhui_function(GtkButton* button,gpointer user_data)
{
	gtk_widget_show_all(window_main);
	gtk_widget_hide_all(window_fuwu);
}
void button_fanhui_zhuanzhang(GtkButton* button,gpointer user_data)
{
	zhuanzhang_clean();
	gtk_widget_show_all(window_fuwu);
	gtk_widget_hide_all(window_zhuanzhang);
}
void button_fanhui_yue(GtkButton* button,gpointer user_data)
{
	gtk_widget_show_all(window_fuwu);
	gtk_widget_hide_all(window_yue);
}
void button_fanhui_gaimi(GtkButton* button,gpointer user_data)
{
	gaimi_clean();
	gtk_widget_show_all(window_fuwu);
	gtk_widget_hide_all(window_gaimi);
}
void button_fanhui_cunkuan(GtkButton* button,gpointer user_data)
{
	cunkuan_clean();
	gtk_widget_show_all(window_fuwu);
	gtk_widget_hide_all(window_cunkuan);
}
void button_fanhui_qukuan(GtkButton* button,gpointer user_data)
{
	qukuan_clean();
	gtk_widget_show_all(window_fuwu);
	gtk_widget_hide_all(window_qukuan);
}
//   退卡回调函数
void button_tuika_fuwu(GtkButton* button,gpointer user_data)
{
	gtk_widget_show_all(window_tuika);
	gtk_widget_hide_all(window_fuwu);
}
void button_tuika_yue(GtkButton* button,gpointer user_data)
{
	gtk_widget_show_all(window_tuika);
	gtk_widget_hide_all(window_yue);
}
void button_tuika_zhuanzhang(GtkButton* button,gpointer user_data)
{
	gtk_widget_show_all(window_tuika);
	gtk_widget_hide_all(window_zhuanzhang);
}
void button_tuika_gaimi(GtkButton* button,gpointer user_data)
{
	gtk_widget_show_all(window_tuika);
	gtk_widget_hide_all(window_gaimi);
}
void button_tuika_cunkuan(GtkButton* button,gpointer user_data)
{
	gtk_widget_show_all(window_tuika);
	gtk_widget_hide_all(window_cunkuan);
}
void button_tuika_qukuan(GtkButton* button,gpointer user_data)
{
	gtk_widget_show_all(window_tuika);
	gtk_widget_hide_all(window_qukuan);
}
void button_fanhui_qita(GtkButton* button,gpointer user_data)
{
	gtk_widget_show_all(window_fuwu);
	gtk_widget_hide_all(window_qita);
}
/*----------登录按钮回调函数----------*/
void button_queren_login(GtkButton* button,gpointer user_data)
{
	gtk_widget_show_all(window_main);
	gtk_widget_hide_all(window_tuika);
}
/*----------其他业务按钮回调函数----------*/
void button_qita_function(GtkButton* button,gpointer user_data)
{
	gtk_widget_show_all(window_qita);
	gtk_widget_hide_all(window_fuwu);
}
/*----------余额查询回调函数----------*/
void button_chaxun_function(GtkButton* button,gpointer user_data)
{

	sqlite3*db;
	sqlite3_open("bugbank.db",&db);
	char sql_chaxun[100] = "";
	sprintf(sql_chaxun,"select yue from bank where card_num = \'%s\';",kahao_char);
	char **table = NULL;
	int r =0,c = 0;
	char *errmsg = NULL;
	sqlite3_get_table(db,sql_chaxun,&table,&r,&c,&errmsg);

	gtk_label_set_text(GTK_LABEL(label_kahaos),kahao_char);
	gtk_label_set_text(GTK_LABEL(label_yues),table[1]);
	gtk_label_set_text(GTK_LABEL(label_uyues),table[1]);

	gtk_widget_show_all(window_yue);
	gtk_widget_hide_all(window_fuwu);
	sqlite3_close(db);
}
/*----------改密按钮回调函数----------*/
void button_gaimi_function(GtkButton* button,gpointer user_data)
{
	gtk_label_set_text(GTK_LABEL(label_card_num_gaimi),kahao_char);
	gtk_widget_show_all(window_gaimi);
	gtk_widget_hide_all(window_fuwu);
}
/*----------改密确认回调函数----------*/
void gaimi_queren(GtkButton* button,gpointer user_data)
{
	const char * yuanmima= gtk_entry_get_text(GTK_ENTRY(entry_yuanmima));
	const char * xinmima= gtk_entry_get_text(GTK_ENTRY(entry_xinmima));
	const char * querenxinmima= gtk_entry_get_text(GTK_ENTRY(entry_querenxinmima));
	sqlite3*db;
	sqlite3_open("bugbank.db",&db);
	char sql_gaimi[100] = "";
	sprintf(sql_gaimi,"select *from bank where card_num = \'%s\'and passwd = \'%s\';",kahao_char,yuanmima);
	char **table = NULL;
	int r =0,c =0;
	char *errmsg = NULL;
	sqlite3_get_table(db,sql_gaimi,&table,&r,&c,&errmsg);
	if(r!=0)
	{
		if (strcmp(xinmima,querenxinmima) == 0)
		{
			char sql_gaimi2[100] = "";
			sprintf(sql_gaimi2,"update bank set passwd = \'%s\' where card_num = \'%s\';",xinmima,kahao_char);
			char *errmsg = NULL;
			sqlite3_exec(db,sql_gaimi2,NULL,NULL,&errmsg);
			sqlite3_close(db);
			gaimi_clean();
			gtk_entry_set_text(GTK_ENTRY(entry_yuanmima),"修改成功！");
		}
		else
		{
			gaimi_clean();
			gtk_entry_set_text(GTK_ENTRY(entry_querenxinmima),"两次密码不一致");
			sqlite3_close(db);
		}	
		return;
	}
	else 
	{
		gaimi_clean();
		gtk_entry_set_text(GTK_ENTRY(entry_yuanmima),"原密码错误，请重新输入");
	}
	return;
}
/*----------转账按钮回调函数----------*/
void button_zhuanzhang_function(GtkButton* button,gpointer user_data)
{
	sqlite3*db;
	sqlite3_open("bugbank.db",&db);
	char sql_chaxun[100] = "";
	sprintf(sql_chaxun,"select yue from bank where card_num = \'%s\';",kahao_char);
	char **table = NULL;
	int r =0,c = 0;
	char *errmsg = NULL;
	sqlite3_get_table(db,sql_chaxun,&table,&r,&c,&errmsg);

	gtk_label_set_text(GTK_LABEL(label_card_num_zhuanzhang),kahao_char);
	gtk_label_set_text(GTK_LABEL(label_zhuanzhang_yue),table[1]);

	gtk_widget_show_all(window_zhuanzhang);
	gtk_widget_hide_all(window_fuwu);
	sqlite3_close(db);
}
/*----------存款按钮回调函数----------*/
void button_cunkuan_function(GtkButton* button,gpointer user_data)
{
	sqlite3*db;
	sqlite3_open("bugbank.db",&db);
	char sql_chaxun[100] = "";
	sprintf(sql_chaxun,"select yue from bank where card_num = \'%s\';",kahao_char);
	char **table = NULL;
	int r =0,c = 0;
	char *errmsg = NULL;
	sqlite3_get_table(db,sql_chaxun,&table,&r,&c,&errmsg);

	gtk_label_set_text(GTK_LABEL(label_card_num_num),kahao_char);
	gtk_label_set_text(GTK_LABEL(label_cunkuan_yue),table[1]);

	gtk_widget_show_all(window_cunkuan);
	gtk_widget_hide_all(window_fuwu);
	sqlite3_close(db);
}
/*----------存款确认按钮回调函数----------*/
void cunkuan_queren(GtkButton* button,gpointer user_data)
{
	const char * jine = gtk_entry_get_text(GTK_ENTRY(entry_cunkuanjine));
	sqlite3*db;
	sqlite3_open("bugbank.db",&db);
	char sql_cunkuan[100] = "";
	sprintf(sql_cunkuan,"select yue from bank where card_num = \'%s\';",kahao_char);
	char **table = NULL;
	int r =0,c = 0;
	char *errmsg = NULL;
	sqlite3_get_table(db,sql_cunkuan,&table,&r,&c,&errmsg);
	if(r != 0)
	{
		int jine_new = atoi(table[1])+ atoi(jine);
		char jine_label[20] = "";
		sprintf(jine_label,"%d",jine_new);//又变回字符串

		gtk_label_set_text(GTK_LABEL(label_cunkuan_yue),jine_label);
		char sql_cunkuan2[100] = "";
		sprintf(sql_cunkuan2,"update bank set yue = %d where card_num = \'%s\';",jine_new,kahao_char);
		errmsg = NULL;
		sqlite3_exec(db,sql_cunkuan2,NULL,NULL,&errmsg);
		sqlite3_close(db);
		cunkuan_clean();
		if(errmsg!=NULL)
		{
			printf("%s\n", errmsg);
		}		
	}
	else
		gtk_label_set_text(GTK_LABEL(label_cunkuan_yue),"存款失败");
		sqlite3_close(db);
	return;
}
/*----------取款按钮回调函数----------*/
void button_qukuan_function(GtkButton* button,gpointer user_data)
{

	sqlite3*db;
	sqlite3_open("bugbank.db",&db);
	char sql_chaxun[100] = "";
	sprintf(sql_chaxun,"select yue from bank where card_num = \'%s\';",kahao_char);
	char **table = NULL;
	int r =0,c = 0;
	char *errmsg = NULL;
	sqlite3_get_table(db,sql_chaxun,&table,&r,&c,&errmsg);

	gtk_label_set_text(GTK_LABEL(label_card_num_qukuan),kahao_char);
	gtk_label_set_text(GTK_LABEL(label_qukuan_yue),table[1]);

	gtk_widget_show_all(window_qukuan);
	gtk_widget_hide_all(window_fuwu);
	sqlite3_close(db);	
}
/*----------取款确认按钮回调函数----------*/
void qukuan_queren(GtkButton* button,gpointer user_data)
{
	const char * jine = gtk_entry_get_text(GTK_ENTRY(entry_qukuanjine));
	sqlite3*db;
	sqlite3_open("bugbank.db",&db);
	char sql_qukuan[100] = "";
	sprintf(sql_qukuan,"select yue from bank where card_num = \'%s\';",kahao_char);
	char **table = NULL;
	int r =0,c = 0;
	char *errmsg = NULL;
	sqlite3_get_table(db,sql_qukuan,&table,&r,&c,&errmsg);
	if(r != 0)
	{
		int jine_new = atoi(table[1]) - atoi(jine);
		char jine_label[20] = "";
		sprintf(jine_label,"%d",jine_new);//又变回字符串

		gtk_label_set_text(GTK_LABEL(label_qukuan_yue),jine_label);
		char sql_qukuan2[100] = "";
		sprintf(sql_qukuan2,"update bank set yue = %d where card_num = \'%s\';",jine_new,kahao_char);
		errmsg = NULL;
		sqlite3_exec(db,sql_qukuan2,NULL,NULL,&errmsg);
		sqlite3_close(db);
		qukuan_clean();
		if(errmsg!=NULL)
		{
			printf("%s\n", errmsg);
		}	
	}
	else
		gtk_label_set_text(GTK_LABEL(label_cunkuan_yue),"取款失败");
		sqlite3_close(db);
	return;
}
/*----------转账确认按钮回调函数----------*/
void zhuanzhang_queren(GtkButton* button,gpointer user_data)
{
	const char * jine = gtk_entry_get_text(GTK_ENTRY(entry_card_zhuanzhang));
	const char * num2 = gtk_entry_get_text(GTK_ENTRY(entry_card_num2));
	sqlite3*db;
	sqlite3_open("bugbank.db",&db);
	char sql[100] = "";
	sprintf(sql,"select *from bank where card_num = \'%s\';",num2);
	char **table = NULL;
	int r =0,c =0;
	char *errmsg = NULL;
	sqlite3_get_table(db,sql,&table,&r,&c,&errmsg);
	if(r!=0)
	{
		char sql2[100] = "";
		sprintf(sql2,"select yue from bank where card_num = \'%s\';",kahao_char);
		char **table = NULL;
		int r =0,c = 0;
		char *errmsg = NULL;
		sqlite3_get_table(db,sql2,&table,&r,&c,&errmsg);
		if(r!=0)
		{
			int jine_new = atoi(table[1])-atoi(jine);
			int jine_new1 = atoi(table[1])+atoi(jine);
			char jine_label[20] = "";
			sprintf(jine_label,"%d",jine_new);//又变回字符串
			gtk_label_set_text(GTK_LABEL(label_zhuanzhang_yue),jine_label);

			char sql2[100] = "";
			sprintf(sql2,"update bank set yue = %d where card_num = \'%s\';",jine_new,kahao_char);
			errmsg = NULL;
			sqlite3_exec(db,sql2,NULL,NULL,&errmsg);

			char sql3[100] = "";
			sprintf(sql3,"update bank set yue = %d where card_num = \'%s\';",jine_new1,num2);
			errmsg = NULL;
			sqlite3_exec(db,sql3,NULL,NULL,&errmsg);
			sqlite3_close(db);
			zhuanzhang_clean();
			gtk_entry_set_text(GTK_ENTRY(entry_card_zhuanzhang),"转账成功！");
		}
		else
		{
			zhuanzhang_clean();
			gtk_entry_set_text(GTK_ENTRY(entry_card_zhuanzhang),"转账错误");
		}
		sqlite3_close(db);
		return;
	}
	else
		zhuanzhang_clean();
		gtk_entry_set_text(GTK_ENTRY(entry_card_num2),"转账卡号错误，请重新输入");
		sqlite3_close(db);
	return;
}
/*----------欢迎界面初始化----------*/
void window_main_init(void)
{
	window_main = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width(GTK_CONTAINER(window_main),20);
	gtk_window_set_title(GTK_WINDOW(window_main),"ATM存取款系统");	//标题
	gtk_window_set_position(GTK_WINDOW(window_main), GTK_WIN_POS_CENTER);	//居中显示
	gtk_widget_set_size_request(window_main,800,480);	//设置窗口大小
	chang_background(window_main,800,480,"image/zhuye.png");	//主窗口背景
 	gtk_window_set_resizable(GTK_WINDOW(window_main), FALSE);	//窗口不可拉伸
	GtkWidget* table = gtk_table_new(6,4,TRUE);
	gtk_container_add(GTK_CONTAINER(window_main),table);
	GtkWidget* card_num = gtk_label_new("卡号：");
	set_widget_font_size(card_num, 20, FALSE);
	gtk_table_attach_defaults(GTK_TABLE(table),card_num,0,1,3,4);

	entry_card_num = gtk_entry_new();			//输入银行卡号
	set_widget_font_size(entry_card_num, 14, FALSE);	//设置字体大小
	gtk_table_attach_defaults(GTK_TABLE(table),entry_card_num,1,3,3,4);

	GtkWidget* passwd = gtk_label_new("密码：");
	set_widget_font_size(passwd, 20, FALSE);
	gtk_table_attach_defaults(GTK_TABLE(table),passwd,0,1,4,5);

	entry_passwd = gtk_entry_new();			//输入密码
	set_widget_font_size(entry_passwd, 14, FALSE);	//设置字体大小
	gtk_table_attach_defaults(GTK_TABLE(table),entry_passwd,1,3,4,5);

	GtkWidget *button_login = gtk_button_new(); // 创建确定按钮
	GtkWidget *image_queding = gtk_image_new_from_file("image/queding.png"); // 图像控件
	gtk_button_set_image(GTK_BUTTON(button_login), image_queding);
	gtk_table_attach_defaults(GTK_TABLE(table),button_login,1,2,5,6);	//把按钮放进布局里
	gtk_button_set_relief(GTK_BUTTON(button_login), GTK_RELIEF_NONE); // 按钮背景色透明
	g_signal_connect(button_login,"pressed",G_CALLBACK(button_login_function),NULL);	//登录按钮信号链接

	GtkWidget *button_empty = gtk_button_new(); // 创建刷新空按钮
	GtkWidget *image_shuaxin = gtk_image_new_from_file("image/shuaxin.png"); // 图像控件
	gtk_button_set_image(GTK_BUTTON(button_empty), image_shuaxin);
	gtk_table_attach_defaults(GTK_TABLE(table),button_empty,2,3,5,6);
	gtk_button_set_relief(GTK_BUTTON(button_empty), GTK_RELIEF_NONE);
	g_signal_connect(button_empty,"pressed",G_CALLBACK(login_clean),NULL);	//刷新按钮信号链接

	GtkWidget *button_shuaka = gtk_button_new(); // 创建刷卡空按钮
	GtkWidget *image_shuaka = gtk_image_new_from_file("image/shuaka.png"); // 图像控件
	gtk_button_set_image(GTK_BUTTON(button_shuaka), image_shuaka);
	gtk_table_attach_defaults(GTK_TABLE(table),button_shuaka,3,4,5,6);
	gtk_button_set_relief(GTK_BUTTON(button_shuaka), GTK_RELIEF_NONE);
	g_signal_connect(button_shuaka,"pressed",G_CALLBACK(shuaka_function),NULL);	//刷卡按钮信号链接

	g_signal_connect(window_main,"destroy",G_CALLBACK(gtk_main_quit),NULL);	//按×自动结束程序
}
/*----------服务界面----------*/
void window_fuwu_init(void)
{
	window_fuwu = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width(GTK_CONTAINER(window_fuwu),20);
	gtk_window_set_title(GTK_WINDOW(window_fuwu),"服务选择界面");	//标题
	gtk_window_set_position(GTK_WINDOW(window_fuwu), GTK_WIN_POS_CENTER);	//居中显示
	gtk_widget_set_size_request(window_fuwu,800,480);
	chang_background(window_fuwu,800,480,"image/bg0.png");
	gtk_window_set_resizable(GTK_WINDOW(window_fuwu), FALSE);	//窗口不可拉伸
	GtkWidget* table = gtk_table_new(7,4,TRUE);
	gtk_container_add(GTK_CONTAINER(window_fuwu),table);

	GtkWidget* button_chaxun= gtk_button_new();		//创建余额按钮
	GtkWidget *image_chaxun = gtk_image_new_from_file("image/yue.png"); // 图像控件
	gtk_button_set_image(GTK_BUTTON(button_chaxun), image_chaxun);
	gtk_table_attach_defaults(GTK_TABLE(table),button_chaxun,0,1,3,4);
	gtk_button_set_relief(GTK_BUTTON(button_chaxun), GTK_RELIEF_NONE);
	g_signal_connect(button_chaxun,"pressed",G_CALLBACK(button_chaxun_function),NULL);	//查询余额按钮信号链接

	GtkWidget* button_gaimi = gtk_button_new();		//创建修改密码按钮
	GtkWidget *image_gaimi = gtk_image_new_from_file("image/gaimi.png"); // 图像控件
	gtk_button_set_image(GTK_BUTTON(button_gaimi), image_gaimi);
	gtk_table_attach_defaults(GTK_TABLE(table),button_gaimi,0,1,4,5);
	gtk_button_set_relief(GTK_BUTTON(button_gaimi), GTK_RELIEF_NONE);
	g_signal_connect(button_gaimi,"pressed",G_CALLBACK(button_gaimi_function),NULL);	//修改密码按钮信号链接

	GtkWidget* button_qita = gtk_button_new();		//创建其他业务按钮
	GtkWidget *image_qita = gtk_image_new_from_file("image/qita.png"); // 图像控件
	gtk_button_set_image(GTK_BUTTON(button_qita), image_qita);
	gtk_table_attach_defaults(GTK_TABLE(table),button_qita,0,1,5,6);
	gtk_button_set_relief(GTK_BUTTON(button_qita), GTK_RELIEF_NONE);
	g_signal_connect(button_qita,"pressed",G_CALLBACK(button_qita_function),NULL);	//其他业务按钮信号链接

	GtkWidget *button_fanhui = gtk_button_new(); // 创建返回按钮
	GtkWidget *image_fanhui = gtk_image_new_from_file("image/fanhui.png"); // 图像控件
	gtk_button_set_image(GTK_BUTTON(button_fanhui), image_fanhui);
	gtk_table_attach_defaults(GTK_TABLE(table),button_fanhui,0,1,6,7);
	gtk_button_set_relief(GTK_BUTTON(button_fanhui), GTK_RELIEF_NONE);
	g_signal_connect(button_fanhui,"pressed",G_CALLBACK(button_fanhui_function),NULL);//返回按钮信号链接

	GtkWidget* button_cunkuan = gtk_button_new();		//创建存款按钮
	GtkWidget *image_cunkuan = gtk_image_new_from_file("image/cunkuan.png"); // 图像控件
	gtk_button_set_image(GTK_BUTTON(button_cunkuan), image_cunkuan);
	gtk_table_attach_defaults(GTK_TABLE(table),button_cunkuan,3,4,3,4);
	gtk_button_set_relief(GTK_BUTTON(button_cunkuan), GTK_RELIEF_NONE);
	g_signal_connect(button_cunkuan,"pressed",G_CALLBACK(button_cunkuan_function),NULL);//返回按钮信号链接

	GtkWidget* button_qukuan = gtk_button_new();		//创建取款按钮
	GtkWidget *image_qukuan = gtk_image_new_from_file("image/qukuan.png"); // 图像控件
	gtk_button_set_image(GTK_BUTTON(button_qukuan), image_qukuan);
	gtk_table_attach_defaults(GTK_TABLE(table),button_qukuan,3,4,4,5);
	gtk_button_set_relief(GTK_BUTTON(button_qukuan), GTK_RELIEF_NONE);
	g_signal_connect(button_qukuan,"pressed",G_CALLBACK(button_qukuan_function),NULL);//返回按钮信号链接

	GtkWidget* button_zhuanzhang = gtk_button_new();	//创建转账按钮
	GtkWidget *image_zhuanzhang = gtk_image_new_from_file("image/zhuanzhang.png"); // 图像控件
	gtk_button_set_image(GTK_BUTTON(button_zhuanzhang), image_zhuanzhang);
	gtk_table_attach_defaults(GTK_TABLE(table),button_zhuanzhang,3,4,5,6);
	gtk_button_set_relief(GTK_BUTTON(button_zhuanzhang), GTK_RELIEF_NONE);
	g_signal_connect(button_zhuanzhang,"pressed",G_CALLBACK(button_zhuanzhang_function),NULL);

	GtkWidget *button_tuika = gtk_button_new(); // 创建退卡按钮
	GtkWidget *image_tuika = gtk_image_new_from_file("image/tuika.png"); // 图像控件
	gtk_button_set_image(GTK_BUTTON(button_tuika), image_tuika);
	gtk_table_attach_defaults(GTK_TABLE(table),button_tuika,3,4,6,7);
	gtk_button_set_relief(GTK_BUTTON(button_tuika), GTK_RELIEF_NONE);
	g_signal_connect(button_tuika,"pressed",G_CALLBACK(button_tuika_fuwu),NULL);//退卡按钮信号链接

	g_signal_connect(window_fuwu,"destroy",G_CALLBACK(gtk_main_quit),NULL);	//按×自动结束程序
}
/*----------转账界面----------*/
void window_zhuanzhang_init(void)
{
    window_zhuanzhang = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width(GTK_CONTAINER(window_zhuanzhang),20);
	gtk_window_set_title(GTK_WINDOW(window_zhuanzhang),"转账");	//标题
	gtk_window_set_position(GTK_WINDOW(window_zhuanzhang), GTK_WIN_POS_CENTER);	//居中显示
	gtk_widget_set_size_request(window_zhuanzhang,800,480);	//设置窗口大小
	chang_background(window_zhuanzhang,800,480,"image/bg3.png");	//主窗口背景
 	gtk_window_set_resizable(GTK_WINDOW(window_zhuanzhang), FALSE);	//窗口不可拉伸
	GtkWidget* table = gtk_table_new(6,4,TRUE);

	gtk_container_add(GTK_CONTAINER(window_zhuanzhang),table);
	GtkWidget* label_card_num = gtk_label_new("你的账号：");
	set_widget_font_size(label_card_num, 16, FALSE);	//设置字体大小
	gtk_table_attach_defaults(GTK_TABLE(table),label_card_num,0,1,1,2);
	
	GtkWidget* label_card_num2 = gtk_label_new("转帐账号：");
	set_widget_font_size(label_card_num2, 16, FALSE);	//设置字体大小
	gtk_table_attach_defaults(GTK_TABLE(table),label_card_num2,0,1,2,3);

	GtkWidget* label_card_zhuanzhang_jine = gtk_label_new("转账金额：");
	set_widget_font_size(label_card_zhuanzhang_jine, 16, FALSE);	//设置字体大小
	gtk_table_attach_defaults(GTK_TABLE(table),label_card_zhuanzhang_jine,0,1,3,4);

	GtkWidget* label_yue = gtk_label_new("您的余额：");
	set_widget_font_size(label_yue, 16, FALSE);	//设置字体大小
	gtk_table_attach_defaults(GTK_TABLE(table),label_yue,0,1,4,5);

	GtkWidget *button_queren = gtk_button_new(); // 创建确认按钮
	GtkWidget *image_queren = gtk_image_new_from_file("image/queren.png"); // 图像控件
	gtk_button_set_image(GTK_BUTTON(button_queren), image_queren);
	gtk_table_attach_defaults(GTK_TABLE(table),button_queren,3,4,1,2);
	gtk_button_set_relief(GTK_BUTTON(button_queren), GTK_RELIEF_NONE);
	g_signal_connect(button_queren,"pressed",G_CALLBACK(zhuanzhang_queren),NULL);

	GtkWidget *button_qingkong = gtk_button_new(); // 创建清空按钮
	GtkWidget *image_qingkong = gtk_image_new_from_file("image/qingkong.png"); // 图像控件
	gtk_button_set_image(GTK_BUTTON(button_qingkong), image_qingkong);
	gtk_table_attach_defaults(GTK_TABLE(table),button_qingkong,3,4,2,3);
	gtk_button_set_relief(GTK_BUTTON(button_qingkong), GTK_RELIEF_NONE);
	g_signal_connect(button_qingkong,"pressed",G_CALLBACK(zhuanzhang_clean),NULL);

	GtkWidget *button_fanhui = gtk_button_new(); // 创建返回按钮
	GtkWidget *image_fanhui = gtk_image_new_from_file("image/fanhui.png"); // 图像控件
	gtk_button_set_image(GTK_BUTTON(button_fanhui), image_fanhui);
	gtk_table_attach_defaults(GTK_TABLE(table),button_fanhui,3,4,3,4);
	gtk_button_set_relief(GTK_BUTTON(button_fanhui), GTK_RELIEF_NONE);
	g_signal_connect(button_fanhui,"pressed",G_CALLBACK(button_fanhui_zhuanzhang),NULL);

	GtkWidget *button_tuika = gtk_button_new(); // 创建退卡按钮
	GtkWidget *image_tuika = gtk_image_new_from_file("image/tuika.png"); // 图像控件
	gtk_button_set_image(GTK_BUTTON(button_tuika), image_tuika);
	gtk_table_attach_defaults(GTK_TABLE(table),button_tuika,3,4,4,5);
	gtk_button_set_relief(GTK_BUTTON(button_tuika), GTK_RELIEF_NONE);
	g_signal_connect(button_tuika,"pressed",G_CALLBACK(button_tuika_zhuanzhang),NULL);//退卡按钮信号链接

	label_card_num_zhuanzhang = gtk_label_new("");	//卡号
	set_widget_font_size(label_card_num_zhuanzhang, 16, FALSE);	//设置字体大小
	gtk_table_attach_defaults(GTK_TABLE(table),label_card_num_zhuanzhang,1,3,1,2);

	entry_card_num2 = gtk_entry_new();		//对方账号
	set_widget_font_size(entry_card_num2, 16, FALSE);	//设置字体大小
	gtk_table_attach_defaults(GTK_TABLE(table),entry_card_num2,1,3,2,3);

	entry_card_zhuanzhang = gtk_entry_new();		//转入金额
	set_widget_font_size(entry_card_zhuanzhang, 16, FALSE);	//设置字体大小
	gtk_table_attach_defaults(GTK_TABLE(table),entry_card_zhuanzhang,1,3,3,4);

	label_zhuanzhang_yue = gtk_label_new("");	//测试余额，以后修改
	set_widget_font_size(label_zhuanzhang_yue, 16, FALSE);	//设置字体大小
	gtk_table_attach_defaults(GTK_TABLE(table),label_zhuanzhang_yue,1,3,4,5);

	g_signal_connect(window_zhuanzhang,"destroy",G_CALLBACK(gtk_main_quit),NULL);	//按×自动结束程序
}
/*----------余额界面----------*/
void window_yue_init(void)
{
    window_yue = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width(GTK_CONTAINER(window_yue),20);
	gtk_window_set_title(GTK_WINDOW(window_yue),"ATM存取款系统");	//标题
	gtk_window_set_position(GTK_WINDOW(window_yue), GTK_WIN_POS_CENTER);	//居中显示
	gtk_widget_set_size_request(window_yue,800,480);	//设置窗口大小
	chang_background(window_yue,800,480,"image/bg3.png");	//主窗口背景
 	gtk_window_set_resizable(GTK_WINDOW(window_yue), FALSE);	//窗口不可拉伸
	
	GtkWidget* table = gtk_table_new(5,4,TRUE);
	gtk_container_add(GTK_CONTAINER(window_yue),table);

	GtkWidget* label_kahao = gtk_label_new("银行卡号：");
	set_widget_font_size(label_kahao, 16, FALSE);
	gtk_table_attach_defaults(GTK_TABLE(table),label_kahao,0,1,1,2);
	GtkWidget* label_yue = gtk_label_new("余额：");
	set_widget_font_size(label_yue, 16, FALSE);
	gtk_table_attach_defaults(GTK_TABLE(table),label_yue,0,1,2,3);
	GtkWidget* label_uyue = gtk_label_new("可用余额：");
	set_widget_font_size(label_uyue, 16, FALSE);
	gtk_table_attach_defaults(GTK_TABLE(table),label_uyue,0,1,3,4);

	label_kahaos = gtk_label_new("");
	set_widget_font_size(label_kahaos, 16, FALSE);
	gtk_table_attach_defaults(GTK_TABLE(table),label_kahaos,1,3,1,2);

	label_yues = gtk_label_new("");
	set_widget_font_size(label_yues, 16, FALSE);
	gtk_table_attach_defaults(GTK_TABLE(table),label_yues,1,3,2,3);

	label_uyues = gtk_label_new("");
	set_widget_font_size(label_uyues, 16, FALSE);
	gtk_table_attach_defaults(GTK_TABLE(table),label_uyues,1,3,3,4);

	GtkWidget* button_fanhui = gtk_button_new();		//创建返回按钮
	GtkWidget *image_fanhui = gtk_image_new_from_file("image/fanhui.png"); // 图像控件
	gtk_button_set_image(GTK_BUTTON(button_fanhui), image_fanhui);
	gtk_table_attach_defaults(GTK_TABLE(table),button_fanhui,3,4,1,2);
	gtk_button_set_relief(GTK_BUTTON(button_fanhui), GTK_RELIEF_NONE);
	g_signal_connect(button_fanhui,"pressed",G_CALLBACK(button_fanhui_yue),NULL);//返回按钮信号链接

	GtkWidget* button_tuika = gtk_button_new();		//创建退卡按钮
	GtkWidget *image_tuika = gtk_image_new_from_file("image/tuika.png"); // 图像控件
	gtk_button_set_image(GTK_BUTTON(button_tuika), image_tuika);
	gtk_table_attach_defaults(GTK_TABLE(table),button_tuika,3,4,3,4);
	gtk_button_set_relief(GTK_BUTTON(button_tuika), GTK_RELIEF_NONE);
	g_signal_connect(button_tuika,"pressed",G_CALLBACK(button_tuika_yue),NULL);//退卡按钮信号链接

	g_signal_connect(window_yue,"destroy",G_CALLBACK(gtk_main_quit),NULL);	//按×自动结束程序
}
/*----------退卡界面----------*/
void window_tuika_init(void)
{
	window_tuika = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width(GTK_CONTAINER(window_tuika),20);
	gtk_window_set_title(GTK_WINDOW(window_tuika),"请收好您的卡片");	//标题
	gtk_window_set_position(GTK_WINDOW(window_tuika), GTK_WIN_POS_CENTER);	//居中显示
	gtk_widget_set_size_request(window_tuika,800,480);	//设置窗口大小
	chang_background(window_tuika,800,480,"image/tuika_bg.png");	//主窗口背景
 	gtk_window_set_resizable(GTK_WINDOW(window_tuika), FALSE);	//窗口不可拉伸
 	GtkWidget* table = gtk_table_new(3,3,TRUE);
	gtk_container_add(GTK_CONTAINER(window_tuika),table);

	GtkWidget* button_queren = gtk_button_new();		//创建确认按钮
	GtkWidget *image_queren= gtk_image_new_from_file("image/queren.png"); // 图像控件
	gtk_button_set_image(GTK_BUTTON(button_queren), image_queren);
	gtk_table_attach_defaults(GTK_TABLE(table),button_queren,1,2,2,3);
	gtk_button_set_relief(GTK_BUTTON(button_queren), GTK_RELIEF_NONE);
	g_signal_connect(button_queren,"pressed",G_CALLBACK(button_queren_login),NULL);//返回按钮信号链接

	g_signal_connect(window_tuika,"destroy",G_CALLBACK(gtk_main_quit),NULL);	//按×自动结束程序
}
/*----------改密界面----------*/
 void window_gaimi_init(void)
{
	window_gaimi = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width(GTK_CONTAINER(window_gaimi),20);
	gtk_window_set_title(GTK_WINDOW(window_gaimi),"修改密码");	//标题
	gtk_window_set_position(GTK_WINDOW(window_gaimi), GTK_WIN_POS_CENTER);	//居中显示
	gtk_widget_set_size_request(window_gaimi,800,480);	//设置窗口大小
	chang_background(window_gaimi,800,480,"image/bg3.png");	//主窗口背景
 	gtk_window_set_resizable(GTK_WINDOW(window_gaimi), FALSE);	//窗口不可拉伸

 	GtkWidget* table = gtk_table_new(6,4,TRUE);
	gtk_container_add(GTK_CONTAINER(window_gaimi),table);
 	GtkWidget* lable_kahao = gtk_label_new("银行卡号：");
	set_widget_font_size(lable_kahao, 16, FALSE);
	gtk_table_attach_defaults(GTK_TABLE(table),lable_kahao,0,1,1,2);
	GtkWidget* lable_yuanmima = gtk_label_new("原密码：");
	set_widget_font_size(lable_yuanmima, 16, FALSE);
	gtk_table_attach_defaults(GTK_TABLE(table),lable_yuanmima,0,1,2,3);
	GtkWidget* lable_xinmima = gtk_label_new("新密码：");
	set_widget_font_size(lable_xinmima, 16, FALSE);
	gtk_table_attach_defaults(GTK_TABLE(table),lable_xinmima,0,1,3,4);
	GtkWidget* lable_xinmimas = gtk_label_new("确认密码：");
	set_widget_font_size(lable_xinmimas, 16, FALSE);
	gtk_table_attach_defaults(GTK_TABLE(table),lable_xinmimas,0,1,4,5);

	label_card_num_gaimi = gtk_label_new("");	//卡号
	set_widget_font_size(label_card_num_gaimi, 14, FALSE);	//设置字体大小
	gtk_table_attach_defaults(GTK_TABLE(table),label_card_num_gaimi,1,3,1,2);

	entry_yuanmima = gtk_entry_new();
	set_widget_font_size(entry_yuanmima, 14, FALSE);
	gtk_table_attach_defaults(GTK_TABLE(table),entry_yuanmima,1,3,2,3);
	entry_xinmima= gtk_entry_new();
	set_widget_font_size(entry_xinmima, 14, FALSE);
	gtk_table_attach_defaults(GTK_TABLE(table),entry_xinmima,1,3,3,4);
	entry_querenxinmima= gtk_entry_new();
	set_widget_font_size(entry_querenxinmima, 14, FALSE);
	gtk_table_attach_defaults(GTK_TABLE(table),entry_querenxinmima,1,3,4,5);

	GtkWidget* button_queren = gtk_button_new();		//创建确认按钮
	GtkWidget *image_queren = gtk_image_new_from_file("image/queren.png");
	gtk_button_set_image(GTK_BUTTON(button_queren), image_queren);
	gtk_table_attach_defaults(GTK_TABLE(table),button_queren,3,4,1,2);
	gtk_button_set_relief(GTK_BUTTON(button_queren), GTK_RELIEF_NONE);
	g_signal_connect(button_queren,"pressed",G_CALLBACK(gaimi_queren),NULL);	//改密确认按钮信号链接

	GtkWidget* button_qingkong = gtk_button_new();		//创建清空按钮
	GtkWidget *image_qingkong = gtk_image_new_from_file("image/qingkong.png");
	gtk_button_set_image(GTK_BUTTON(button_qingkong), image_qingkong);
	gtk_table_attach_defaults(GTK_TABLE(table),button_qingkong,3,4,2,3);
	gtk_button_set_relief(GTK_BUTTON(button_qingkong), GTK_RELIEF_NONE);
	g_signal_connect(button_qingkong,"pressed",G_CALLBACK(gaimi_clean),NULL);	//清空按钮信号链接

	GtkWidget* button_fanhui = gtk_button_new();		//创建返回按钮
	GtkWidget *image_fanhui = gtk_image_new_from_file("image/fanhui.png");
	gtk_button_set_image(GTK_BUTTON(button_fanhui), image_fanhui);
	gtk_table_attach_defaults(GTK_TABLE(table),button_fanhui,3,4,3,4);
	gtk_button_set_relief(GTK_BUTTON(button_fanhui), GTK_RELIEF_NONE);
	g_signal_connect(button_fanhui,"pressed",G_CALLBACK(button_fanhui_gaimi),NULL);	//返回按钮信号链接

	GtkWidget* button_tuika = gtk_button_new();			//创建退卡按钮
	GtkWidget *image_tuika = gtk_image_new_from_file("image/tuika.png");
	gtk_button_set_image(GTK_BUTTON(button_tuika), image_tuika);
	gtk_table_attach_defaults(GTK_TABLE(table),button_tuika,3,4,4,5);
	gtk_button_set_relief(GTK_BUTTON(button_tuika), GTK_RELIEF_NONE);
	g_signal_connect(button_tuika,"pressed",G_CALLBACK(button_tuika_gaimi),NULL);//退卡按钮信号链接

	g_signal_connect(window_gaimi,"destroy",G_CALLBACK(gtk_main_quit),NULL);	//按×自动结束程序
}
/*----------存款界面----------*/
void window_cunkuan_init(void)
{
    window_cunkuan = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width(GTK_CONTAINER(window_cunkuan),20);
	gtk_window_set_title(GTK_WINDOW(window_cunkuan),"存款");	//标题
	gtk_window_set_position(GTK_WINDOW(window_cunkuan), GTK_WIN_POS_CENTER);	//居中显示
	gtk_widget_set_size_request(window_cunkuan,800,480);	//设置窗口大小
	chang_background(window_cunkuan,800,480,"image/bg2.png");	//主窗口背景
 	gtk_window_set_resizable(GTK_WINDOW(window_cunkuan), FALSE);	//窗口不可拉伸
	GtkWidget* table = gtk_table_new(5,4,TRUE);

	gtk_container_add(GTK_CONTAINER(window_cunkuan),table);
	GtkWidget* label_kahao = gtk_label_new("银行卡号：");
	set_widget_font_size(label_kahao, 16, FALSE);
	gtk_table_attach_defaults(GTK_TABLE(table),label_kahao,0,1,1,2);
	GtkWidget* label_jine = gtk_label_new("存款金额：");
	set_widget_font_size(label_jine, 16, FALSE);
	gtk_table_attach_defaults(GTK_TABLE(table),label_jine,0,1,2,3);
	GtkWidget* label_yue = gtk_label_new("您的余额为：");
	set_widget_font_size(label_yue, 16, FALSE);
	gtk_table_attach_defaults(GTK_TABLE(table),label_yue,0,1,3,4);

	label_card_num_num = gtk_label_new("");	//卡号
	set_widget_font_size(label_card_num_num, 16, FALSE);	//设置字体大小
	gtk_table_attach_defaults(GTK_TABLE(table),label_card_num_num,1,3,1,2);

	entry_cunkuanjine = gtk_entry_new();
	set_widget_font_size(entry_cunkuanjine, 16, FALSE);
	gtk_table_attach_defaults(GTK_TABLE(table),entry_cunkuanjine,1,3,2,3);

	label_cunkuan_yue = gtk_label_new("");
	set_widget_font_size(label_cunkuan_yue, 16, FALSE);
	gtk_table_attach_defaults(GTK_TABLE(table),label_cunkuan_yue,1,3,3,4);

	GtkWidget* button_queren = gtk_button_new();		//确认按钮
	GtkWidget *image_queren= gtk_image_new_from_file("image/queren.png"); // 图像控件
	gtk_button_set_image(GTK_BUTTON(button_queren), image_queren);
	gtk_table_attach_defaults(GTK_TABLE(table),button_queren,3,4,1,2);
	gtk_button_set_relief(GTK_BUTTON(button_queren), GTK_RELIEF_NONE);
	g_signal_connect(button_queren,"pressed",G_CALLBACK(cunkuan_queren),NULL);	//存款确认按钮信号链接

	GtkWidget* button_qingkong = gtk_button_new();		//创建清空按钮
	GtkWidget *image_qingkong = gtk_image_new_from_file("image/qingkong.png");
	gtk_button_set_image(GTK_BUTTON(button_qingkong), image_qingkong);
	gtk_table_attach_defaults(GTK_TABLE(table),button_qingkong,3,4,2,3);
	gtk_button_set_relief(GTK_BUTTON(button_qingkong), GTK_RELIEF_NONE);
	g_signal_connect(button_qingkong,"pressed",G_CALLBACK(cunkuan_clean),NULL);	//清空按钮信号链接

	GtkWidget* button_fanhui = gtk_button_new();		//返回按钮
	GtkWidget *image_fanhui = gtk_image_new_from_file("image/fanhui.png"); // 图像控件
	gtk_button_set_image(GTK_BUTTON(button_fanhui), image_fanhui);
	gtk_table_attach_defaults(GTK_TABLE(table),button_fanhui,3,4,3,4);
	gtk_button_set_relief(GTK_BUTTON(button_fanhui), GTK_RELIEF_NONE);
	g_signal_connect(button_fanhui,"pressed",G_CALLBACK(button_fanhui_cunkuan),NULL);//返回按钮信号链接

	GtkWidget* button_tuika = gtk_button_new();		//退卡按钮
	GtkWidget *image_tuika = gtk_image_new_from_file("image/tuika.png"); // 图像控件
	gtk_button_set_image(GTK_BUTTON(button_tuika), image_tuika);
	gtk_table_attach_defaults(GTK_TABLE(table),button_tuika,3,4,4,5);
	gtk_button_set_relief(GTK_BUTTON(button_tuika), GTK_RELIEF_NONE);
	g_signal_connect(button_tuika,"pressed",G_CALLBACK(button_tuika_cunkuan),NULL);//退卡按钮信号链接

	g_signal_connect(window_cunkuan,"destroy",G_CALLBACK(gtk_main_quit),NULL);	//按×自动结束程序
}
/*----------取款界面----------*/
void window_qukuan_init(void)
{
    window_qukuan = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width(GTK_CONTAINER(window_qukuan),20);
	gtk_window_set_title(GTK_WINDOW(window_qukuan),"取款");	//标题
	gtk_window_set_position(GTK_WINDOW(window_qukuan), GTK_WIN_POS_CENTER);	//居中显示
	gtk_widget_set_size_request(window_qukuan,800,480);	//设置窗口大小
	chang_background(window_qukuan,800,480,"image/bg2.png");	//主窗口背景
 	gtk_window_set_resizable(GTK_WINDOW(window_qukuan), FALSE);	//窗口不可拉伸
	GtkWidget* table = gtk_table_new(5,4,TRUE);

	gtk_container_add(GTK_CONTAINER(window_qukuan),table);
	GtkWidget* label_kahao = gtk_label_new("银行卡号：");
	set_widget_font_size(label_kahao, 16, FALSE);
	gtk_table_attach_defaults(GTK_TABLE(table),label_kahao,0,1,1,2);
	GtkWidget* label_jine = gtk_label_new("取款金额：");
	set_widget_font_size(label_jine, 16, FALSE);
	gtk_table_attach_defaults(GTK_TABLE(table),label_jine,0,1,2,3);
	GtkWidget* label_yue = gtk_label_new("您的余额为：");
	set_widget_font_size(label_yue, 16, FALSE);
	gtk_table_attach_defaults(GTK_TABLE(table),label_yue,0,1,3,4);

	label_card_num_qukuan = gtk_label_new("");	//卡号
	set_widget_font_size(label_card_num_qukuan, 16, FALSE);	//设置字体大小
	gtk_table_attach_defaults(GTK_TABLE(table),label_card_num_qukuan,1,3,1,2);

	entry_qukuanjine = gtk_entry_new();
	set_widget_font_size(entry_qukuanjine, 16, FALSE);
	gtk_table_attach_defaults(GTK_TABLE(table),entry_qukuanjine,1,3,2,3);

	label_qukuan_yue = gtk_label_new("");
	set_widget_font_size(label_qukuan_yue, 16, FALSE);
	gtk_table_attach_defaults(GTK_TABLE(table),label_qukuan_yue,1,3,3,4);

	GtkWidget* button_queren = gtk_button_new();		//确认按钮
	GtkWidget *image_queren= gtk_image_new_from_file("image/queren.png"); // 图像控件
	gtk_button_set_image(GTK_BUTTON(button_queren), image_queren);
	gtk_table_attach_defaults(GTK_TABLE(table),button_queren,3,4,1,2);
	gtk_button_set_relief(GTK_BUTTON(button_queren), GTK_RELIEF_NONE);
	g_signal_connect(button_queren,"pressed",G_CALLBACK(qukuan_queren),NULL);	//取款确认按钮信号链接

	GtkWidget* button_qingkong = gtk_button_new();		//创建清空按钮
	GtkWidget *image_qingkong = gtk_image_new_from_file("image/qingkong.png");
	gtk_button_set_image(GTK_BUTTON(button_qingkong), image_qingkong);
	gtk_table_attach_defaults(GTK_TABLE(table),button_qingkong,3,4,2,3);
	gtk_button_set_relief(GTK_BUTTON(button_qingkong), GTK_RELIEF_NONE);
	g_signal_connect(button_qingkong,"pressed",G_CALLBACK(qukuan_clean),NULL);	//清空按钮信号链接

	GtkWidget* button_fanhui = gtk_button_new();		//返回按钮
	GtkWidget *image_fanhui = gtk_image_new_from_file("image/fanhui.png"); // 图像控件
	gtk_button_set_image(GTK_BUTTON(button_fanhui), image_fanhui);
	gtk_table_attach_defaults(GTK_TABLE(table),button_fanhui,3,4,3,4);
	gtk_button_set_relief(GTK_BUTTON(button_fanhui), GTK_RELIEF_NONE);
	g_signal_connect(button_fanhui,"pressed",G_CALLBACK(button_fanhui_qukuan),NULL);//返回按钮信号链接

	GtkWidget* button_tuika = gtk_button_new();		//退卡按钮
	GtkWidget *image_tuika = gtk_image_new_from_file("image/tuika.png"); // 图像控件
	gtk_button_set_image(GTK_BUTTON(button_tuika), image_tuika);
	gtk_table_attach_defaults(GTK_TABLE(table),button_tuika,3,4,4,5);
	gtk_button_set_relief(GTK_BUTTON(button_tuika), GTK_RELIEF_NONE);
	g_signal_connect(button_tuika,"pressed",G_CALLBACK(button_tuika_qukuan),NULL);//退卡按钮信号链接

	g_signal_connect(window_qukuan,"destroy",G_CALLBACK(gtk_main_quit),NULL);	//按×自动结束程序
}
/*----------其他业务界面----------*/
void window_qita_init(void)
{
	window_qita = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width(GTK_CONTAINER(window_qita),8);
	gtk_window_set_title(GTK_WINDOW(window_qita),"其他业务");	//标题
	gtk_window_set_position(GTK_WINDOW(window_qita), GTK_WIN_POS_CENTER);	//居中显示
	gtk_widget_set_size_request(window_qita,800,480);	//设置窗口大小
	chang_background(window_qita,800,480,"image/bg1.png");	//主窗口背景
 	gtk_window_set_resizable(GTK_WINDOW(window_qita), FALSE);	//窗口不可拉伸
 	GtkWidget* table = gtk_table_new(6,4,TRUE);

 	gtk_container_add(GTK_CONTAINER(window_qita),table);
 	GtkWidget* button_fanhui = gtk_button_new();		//返回按钮
	GtkWidget *image_fanhui = gtk_image_new_from_file("image/fanhui.png"); // 图像控件
	gtk_button_set_image(GTK_BUTTON(button_fanhui), image_fanhui);
	gtk_table_attach_defaults(GTK_TABLE(table),button_fanhui,3,4,5,6);
	gtk_button_set_relief(GTK_BUTTON(button_fanhui), GTK_RELIEF_NONE);
	g_signal_connect(button_fanhui,"pressed",G_CALLBACK(button_fanhui_qita),NULL);//返回按钮信号链接

	g_signal_connect(window_qita,"destroy",G_CALLBACK(gtk_main_quit),NULL);	//按×自动结束程序	
}
/*----------主函数----------*/
int main(int argc, char *argv[])
{
	gtk_init(&argc,&argv);
	window_main_init();
	window_fuwu_init();
	window_zhuanzhang_init();
	window_yue_init();
	window_tuika_init();
	window_gaimi_init();
	window_cunkuan_init();
	window_qukuan_init();
	window_qita_init();

	gtk_widget_show_all(window_main);
	gtk_main();
	return 0;
}