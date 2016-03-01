#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <inttypes.h>
#include <unistd.h>

#define MAXLEN 80
const char filename[]="data.dat";

typedef unsigned char byte;

struct record{
    uint64_t id;
    char website[MAXLEN];
    char username[MAXLEN];
    char password[MAXLEN];
    char securecode[MAXLEN];
    char other[BUFSIZ];
    time_t createtime;
    time_t updatetime;
};
const int recordnum = 7;

typedef enum{
    NONE,WEBSITE,USERNAME,PASSWORD,SECURECODE,OTHER
}PropertyType;

struct record *initRecord(struct record *val);
struct record *newRecord(struct record *val);
struct record *modfyRecord(struct record *val,PropertyType type);
void recordInfo(struct record *val);
inline void printTime(struct tm *timeinfo,char *buf,size_t len,time_t *t);

void initData(byte *key_data,size_t keysz);

struct record *readRecord(struct record **pprec,uint64_t *sz);
uint64_t writeRecord(struct record *prec,uint64_t sz);

void print(struct record *prec,uint64_t sz);

void store(struct record *val,uint64_t n,struct record *prec,uint64_t *sz);

uint64_t find_first_of(struct record *key,struct record *src,uint64_t sz);
uint64_t insert(struct record *stc,uint64_t *sz,struct record *r,uint64_t pos);
uint64_t modify(struct record *stc,uint64_t *sz,struct record *r,uint64_t pos);
uint64_t delete(struct record *src,uint64_t *sz,struct record *r,uint64_t pos);

void handle();

inline void trimrn(char *s);//trim '\n'

_Bool fileExists()
{
    return access(filename,0)!=-1;
}

int main(int argc,char *argv[])
{
    if(argc<2){
        printf("usage:%s look|insert|modify|delete.\n",argv[0]);
        return -1;
    }
    struct record *p=NULL;
    uint64_t sz=0;

    if(fileExists())
        readRecord(&p,&sz);

    if(strncmp("look",argv[1],4)==0){

        print(p,sz);
    }else if(strncmp("insert",argv[1],5)==0){
        struct record r;

        if(fileExists())
            readRecord(&p,&sz);
        else
            p=(struct record *)malloc(sizeof(struct record));

        newRecord(&r);
        store(&r,1,p,&sz);
        writeRecord(p,sz);
    }else if(strncmp("modify",argv[1],6)==0){
        if(fileExists())
            readRecord(&p,&sz);
        else
            p=(struct record *)malloc(sizeof(struct record));
        struct record r;
        printf("填写查找Key:\n");
        newRecord(&r);
        uint64_t pos = find_first_of(&r,p,sz);
        if(pos!=0){
            PropertyType type=NONE;
            while(type==NONE && type>OTHER){
                printf("修改部分:\n[1]网站\n[2]用户\n[3]密码\n[4]安全码\n[5]备注\n");
                scanf("%u",&type);
            }
            modfyRecord(p+pos,type);
        }else{
            printf("没有找到,输入是否有误.\n");

        }
    }

    if(p!=NULL)
        free(p);
    return 0;
}

//修改
struct record *modfyRecord(struct record *val,PropertyType type)
{
    char *p;
    int input_len=MAXLEN;
    switch(type){
        case WEBSITE:
            p = val->website;
            printf("old:[%s]new website:",p);
            break;
        case USERNAME:
            p = val->username;
            printf("old:[%s]new username:",p);
            break;
        case PASSWORD:
            p = val->password;
            printf("old:[%s]new password:",p);
            break;
        case OTHER:
            p = val->other;
            input_len=BUFSIZ;
            printf("old:[%s]new other:",p);
            break;
        case SECURECODE:
            p = val->securecode;
            printf("old:[%s]new securecode:",p);
            break;
        default:
            printf("modify type is ERROR.\n");
            return val;
    }

    fgets(p,input_len,stdin);
    trimrn(p);
    val->updatetime = time(NULL);
    return val;
}
//初始化
struct record *initRecord(struct record *val)
{
    val->id=0;
    val->updatetime = val->createtime = time(NULL);
    return val;
}
//新record
struct record *newRecord(struct record *val)
{
    //vak->id = ;
    printf("website:");
    fgets(val->website,MAXLEN,stdin);
    trimrn(val->website);

    printf("username:");
    fgets(val->username,MAXLEN,stdin);
    trimrn(val->username);
    printf("password:");
    fgets(val->password,MAXLEN,stdin);
    trimrn(val->password);
    printf("securecode:");
    fgets(val->securecode,MAXLEN,stdin);
    trimrn(val->securecode);
    printf("other:");
    fgets(val->other,BUFSIZ,stdin);
    trimrn(val->other);
    time(&val->createtime);
    time(&val->updatetime);
    return val;
}

void printTime(struct tm *timeinfo,char *buf,size_t len,time_t *t)
{
    timeinfo = localtime(t);
    strftime(buf,len,"%F %X %Z%n",timeinfo);
}

void recordInfo(struct record *val)
{
    printf("---------------------\n");
    printf("网站:%s\n用户:%s\n密码:%s\n安全:%s\n",val->website,val->username,val->password,val->securecode);
    char buf[60];
    printf("备注:%s\n",val->other);
    struct tm *timeinfo;
    printTime(timeinfo,buf,60,&val->createtime);
    printf("创建:%s",buf);
    printTime(timeinfo,buf,60,&val->updatetime);
    printf("修改:%s",buf);
    printf("---------------------\n");
}

//解密文件数据
void initData(byte *key_data,size_t keysz)
{
    /*
    if(fileExists()){
        FILE *fp=fopen(filename,"rb");
        fread();
    }
*/
}
void trimrn(char *s)
{
    int len = strlen(s);
    s[len-1]='\0';
}
struct record *readRecord(struct record **pprec,uint64_t *sz)
{
    FILE *fp = fopen(filename,"rb");
    struct record *ret = NULL;
    if(fp==NULL){
        fprintf(stderr,"cannot found record data!\n");
        goto fallthrough_1;
    }

    size_t len = fread(sz,sizeof(uint64_t),1,fp);
    if(len!=1){
        fprintf(stderr,"read data ERROR!\n");
        goto fallthrough_1;
    }
    *pprec = (struct record*)malloc(sizeof(struct record)*(*sz));
    if(NULL==*pprec){
        fprintf(stderr,"memory ERROR!\n");
        goto fallthrough_1;
    }

    len = fread(*pprec,sizeof(struct record),*sz,fp);
    if(len!=*sz){
        fprintf(stderr,"read count ERROR!\n");
        goto fallthrough_1;
    }
    ret = *pprec;
fallthrough_1:
    fclose(fp);
    return ret;
}

size_t writeRecord(struct record *prec,size_t sz)
{
    FILE *fp = fopen(filename,"wb");
    size_t ret = -1;
    if(fp==NULL){
        fprintf(stderr,"cannot write record data!\n");
        goto fallthrough_2;
    }
    ret = fwrite(&sz,sizeof(uint64_t),1,fp);
    if(ret!=1){
        fprintf(stderr,"write ERROR!\n");
        goto fallthrough_2;
    }
    ret = fwrite(prec,sizeof(struct record),sz,fp);
    if(ret!=sz){
        fprintf(stderr,"write count ERROR!\n");
        goto fallthrough_2;
    }
fallthrough_2:
    fclose(fp);
    return ret;
}

void print(struct record *prec,uint64_t sz)
{
    uint64_t i;
    for(i=0;i<sz;++i){
        printf("序号:%lu\n",i);
        recordInfo(&prec[i]);
    }
}

void store(struct record *val,uint64_t n,struct record *prec,uint64_t *sz)
{
    uint64_t len = *sz;
    prec = (struct record *)realloc(prec,sizeof(struct record)*(len+n));

    if((len+n)>BUFSIZ){
        fprintf(stderr,"records overflow.\n");
    }
    memcpy(prec+*sz,val,sizeof(struct record)*n);
    *sz+=n;
}
//查找
int64_t find_first_of(struct record *key,struct record *src,uint64_t sz)
{
    int64_t i;
    char *keyw = key->website;
    char *keyu = key->username;
    for(i=0;i<sz;++i){
        if(strncmp(keyw,src[i].website,strlen(keyw))==0 || strncmp(keyu,src[i].username,strlen(keyu))==0){
            return i;
        }
    }
    return -1;
}
uint64_t insert(struct record *stc,uint64_t *sz,struct record *r,uint64_t pos)
{
}
uint64_t modify(struct record *stc,uint64_t *sz,struct record *r,uint64_t pos)
{
}
uint64_t delete(struct record *src,uint64_t *sz,struct record *r,uint64_t pos)
{
}
