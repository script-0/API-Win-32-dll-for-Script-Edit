#include <windows.h>
#include <stdio.h>
#include <commctrl.h>
#include <math.h>
#include <locale.h>
#include <commdlg.h>
#include "initial.h"


#ifdef __cplusplus
extern "C"
{
#endif

/**Name:Load_initial_info
 *Object:Load initial informations need to load app
 *Argument: Point to INITIAL_INFO structure
 *Return's Value:  TRUE if all initial informations are being loaded
                    FALSE else
*/
DLL_EXPORT BOOL load_initial_info(INITIAL_INFO* info)
{
    info->x0=(GetSystemMetrics(SM_CXSCREEN)-DIALOG_WIDTH)/2;
    info->y0=(GetSystemMetrics(SM_CYSCREEN) -DIALOG_HEIGHT)/2;
    info->height=DIALOG_HEIGHT-43;
    info->width=DIALOG_WIDTH-375;
    info->state_bar=FALSE;
    info->state_bck_color=FALSE;
    info->opacity=100;
    return TRUE;
}

/**Name:parse
 *Objectifs: extraire les arguments de cline qui est un tableau contenant tous les arguments et les ouvre dans l'app
            L'extraction se fait du premier au dernier.
 *Entrees: cline le tableau d'arguments
           hwnd le HWND de l'app
           caption le pointeur vers la chaine contenant le titre de l'app
           Setfile le pointeur vers les booleans specifiant si un fichier a ete ouvert(Setfile[0]) et si son backup a ete cree(Setfile[1])
 *Sortie:
            rien
 */
DLL_EXPORT void parse(HWND hwnd,char** cline,char* caption,BOOL* Setfile)
{
    char cline_tmp[MAX_PATH]={'\0'},c_tmp[MAX_PATH]={'\0'};
    int i=0,j=0,i_test=0,j_test=0,len=(int)strlen(cline[0]);
    while(j<len)
    {
            while((cline[0][j]!=' ') && (cline[0][j]!='\0'))
            {
                if(cline[0][j]=='"')
                {
                    j_test++;
                    i--;
                }
                else{
                      cline_tmp[i]=cline[0][j];
                    }

                j++;
                i++;
            }
            if(cline[0][j]==' ')
            {
                if(j_test!=0)
                {
                    while(((cline[0][j]!='"') && (j_test!=0)) && (j<len))
                    {
                        if(cline[0][j]!='"')cline_tmp[i]=cline[0][j];
                        else
                        {
                         i--; j_test--;
                        }
                        j++;
                        i++;
                    }
                    j++;
                }
            }
            int c_r=test_filename(cline_tmp);
            switch(c_r)
            {
                  case NO_PATH:
                                strcpy(c_tmp,cline_tmp);
                                GetCurrentDirectory(_MAX_PATH,cline_tmp);
                                strcat(cline_tmp,"\\");
                                strcat(cline_tmp,c_tmp);
                                break;
                  case NO_EXT:
                                break;
                  default:
                            break;
            }
            if(i_test==0)
            {
                    read_drop_file(hwnd,Setfile,cline_tmp);
                    strcpy(caption,cline_tmp);
                    i_test=1;
            }
            else{update_recent_file(hwnd,cline_tmp);}
            i=0;
            for(int u=0;u<MAX_PATH;u++){cline_tmp[u]='\0';}
            j++;
    }

    if(i_test==1){update_recent_file(hwnd,caption);}
}

/**Name:test_filename
 *Objectif: verifie si un chemin d'acces est correcte: Drive+path+filename+ext.  Retourne une valeur en fonction de ce qui manque
 *Entree: le chemin en question
 *Sorties:
            NO_PATH si le chemin n'a pas le drive ou le path valide-
            NO_EXT  si le chemin n'a pas  d'extension valide
            NO_PATH_NO_EXT si le chemin n'a ni drive ou path ni extension valide
 */
DLL_EXPORT int test_filename(const char* c_path)
{
    char drive[3]={'\0'},path[MAX_PATH]={'\0'},filename[MAX_PATH]={'\0'},ext[5]={'\0'};
	_splitpath(c_path,drive,path,filename,ext);
	int result=2;
	if(drive[0]=='\0'){result=NO_PATH; printf("(DLL)--test_filename-- :NO_PATH\n");}
    if(ext[0]=='\0')
    {
       if(result==NO_PATH){result=NO_PATH_NO_EXT ;printf("(DLL)--test_filename-- :NO_EXT\n");}
       else{result=NO_EXT; printf("(DLL)--test_filename-- :NO_PATH_NO_EXT\n");}
    }
    return result;
}

/**Name:last_session
 *Objectifs: demande a l'utilisateur s'il veut reouvrir(et le reouvre) le dernier fichier ouvert si la derniere utilisation de l'app s'est termine brusquement
 *Entrees:
        hwnd le HWND de l'app
        Setfile le pointeur vers les booleans specifiant si un fichier a ete ouvert(Setfile[0]) et si son backup a ete cree(Setfile[1])
 *Sortie:
        rien
 */
DLL_EXPORT void last_session(HWND hwnd,BOOL* Setfile)
{
    char ctext[_MAX_PATH];
    GetTempPathA(_MAX_PATH,ctext);
    strcat(ctext,"last_file.t");
    if(fileExist(ctext))
    {
        if(MessageBox(hwnd,"Your last session ended abrutly.Do you want do reload file?","Note",MB_YESNO|MB_ICONINFORMATION)==IDYES)
        {
            read_drop_file(hwnd,Setfile,ctext);
        }
        DeleteFile(ctext);
    }
}

/**Name:backup_file
 *Objectifs: creer un backup du fichier ouvert (permettant l'annulation de toutes les modificaions apportes au fichier) ou restore le fichier ou supprime le backup file
 *Entrees:
        fname le chemin absolu du fichier ouvert
        flag un booleen  qui defini l'action a effectue, BACKUP -> cree le backup
                                                         RESTORE_FILE -> restore le fichier
                                                         DEL_BACKUP -> supprime le backup file
 *Sorties:
        TRUE si le backup a ete cree avec succes et FALSE sinon
        TRUE si le fichier a ete restore avec succes et FALSE sinon
        TRUE si le backup file a ete supprime avec succes et FALSE sinon
 */
DLL_EXPORT BOOL backup_file(const char* fname,BOOL flag)
{

    char ctext[_MAX_PATH];
    GetTempPathA(_MAX_PATH,ctext);
    strcat(ctext,"tedit_restore.t");
    if(flag==BACKUP)
    {
        if(!CopyFileA(fname,ctext,FALSE))
        {
            MessageBox(NULL,TEXT("Backup file failed. It will be impossible to restore initial file after any modification"),TEXT("Note"),MB_OK|MB_ICONINFORMATION);
            return FALSE;
        }
    }else if(flag==RESTORE_FILE)
    {
            if(!CopyFileA(ctext,fname,FALSE))
                {
                 MessageBox(NULL,"Backup operation failed","Note",MB_OK|MB_ICONERROR);
                 return FALSE;
                }
    }else // DEL_BACKUP case
    {
        if(!SUCCEEDED(DeleteFile(ctext)))
        {
            MessageBox(NULL,"Backup operation failed","Note",MB_OK|MB_ICONERROR);
            return FALSE;
        }
    }
    return TRUE;
 }

/**Name:GetDroppedFilesPaths
 *Objectifs: recupere le nombre de fichiers droppes et leur chemin d'acces respectifs qui seront places dans path
 *Entrees:
            hdrop:
            path:un pointeur vers le tableau qui contiendra les differents chemins d'acces
 *Sortie:
         le nombre de fichiers droppes
 */
DLL_EXPORT UINT GetDroppedFilesPaths(HDROP hdrop,char*** path)
{
    UINT length=0;
    UINT nb_file=DragQueryFile(hdrop,0xFFFFFFFF,NULL,0);

   if(nb_file <=0){return UNKNOWN_ERROR;}
   // *paths=(char**)GobalAlloc(GPTR,nb_file*sizeof(*paths));
    char** paths=(char**)GlobalAlloc(GPTR,nb_file*sizeof(char*));
    if(paths==NULL){return ERROR_MEMORY;}
    for(UINT i=0;i<nb_file;i++)
    {
        length=DragQueryFile(hdrop,i,NULL,0) + 1;
        paths[i]=(char*)GlobalAlloc(GPTR,length*sizeof(char));
        if(paths[i] == NULL )return i+1;
        DragQueryFile(hdrop,i,paths[i],length);
    }
    *path=paths;
    return nb_file;
}

/**Name: read_drop_file
 *Objectif: lie le fichier specifie par fname et fait toute les modifications neccessaires sur la fenetre:
            -Definie la variable Setfile[0] a TRUE
            -Affiche l'addresse du fichier dans la barre de titre et le control edit "FILENAME"
            -Mettre a jour la list box recent file
 *Entrees:
          hwnd: le HWND de l'app
          Setfile: le pointeur vers les booleans specifiant si un fichier a ete ouvert(Setfile[0]) et si son backup a ete cree(Setfile[1])
          fname:le chemin d'acces absolu vers le fichier
 *Sortie:
         TRUE si tout s'est bien passe FALSE sinon
 */
DLL_EXPORT BOOL read_drop_file(HWND hwnd ,BOOL* Setfile,char* fname)
{
    BOOL result=FALSE;
    HANDLE hfile=NULL;
    char* ftext=NULL;
    static DWORD read;
    static DWORD fsize;
    hfile=CreateFile(fname,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
    if(hfile==INVALID_HANDLE_VALUE)
    {           if(fileExist(fname))
                {
                    char* error=NULL;
                    error=(char*)LocalAlloc(GPTR,(MAX_PATH+20)*sizeof(char));
                    if(error!=NULL)
                    {
                        strcpy(error,"File ");
                        strcat(error,fname);
                        strcat(error,":Access denied");
                        MessageBox(NULL,error,TEXT("Error"),MB_OK|MB_ICONERROR);
                       LocalFree(error);
                    }
                    else{ MessageBox(hwnd,TEXT("Error:Access denied..."),TEXT("Error"),MB_OK|MB_ICONWARNING);}
                }
                else{
                        char* error=NULL;
                        error=(char*)LocalAlloc(GPTR,(MAX_PATH+18)*sizeof(char));
                        if(error!=NULL)
                        {
                            strcpy(error,"File ");
                            strcat(error,fname);
                            strcat(error,":invalid file");
                            MessageBox(NULL,error,TEXT("Error"),MB_OK|MB_ICONERROR);
                            LocalFree(error);
                        }
                        else{ MessageBox(hwnd,TEXT("Error: invalid file ..."),TEXT("Error"),MB_OK|MB_ICONWARNING);}
                    }
    }
    else{
                fsize=GetFileSize(hfile,NULL);
                if(fsize == 0xFFFFFFFF)
                {
                    MessageBox(hwnd,TEXT("Error:specified file has invalid\nRessayer..."),TEXT("Error"),MB_OK|MB_ICONWARNING);
                }
                else
                {
                    ftext=(char*)GlobalAlloc(GPTR,(fsize+1)*sizeof(char));
                    if(ftext==NULL)
                    {
                        MessageBox(hwnd,TEXT("Error:Memory access denied.\nRessayer..."),TEXT("Error"),MB_OK|MB_ICONWARNING);
                    }
                    else
                    {
                      if(!ReadFile(hfile,ftext,fsize,&read,NULL))
                      {
                        DWORD dwerror=GetLastError();
                        read_error(hwnd,dwerror);
                      }
                      else{
                      ftext[fsize]='\0';
                      char* posR = strchr(ftext,'\r');
                      char* posN = strchr(ftext,'\n');
                      int nb=0,state;
                      if(posN != NULL && posR==NULL)
                      {
                          for(DWORD u=0;u<fsize;u++)
                          {
                              if(ftext[u]==(char)('\n')) nb++;
                          }
                          state=0;
                      }
                      else if(posN == NULL && posR!=NULL)
                      {
                          for(DWORD u=0;u<fsize;u++)
                          {
                              if(ftext[u]==(char)('\r')) nb++;
                          }
                          state=1;
                      }
                      if(nb>0)
                      {
                          fsize+=nb;
                          char* output=(char*)GlobalAlloc(GPTR,(fsize+1)*sizeof(char));
                          DWORD track=0;
                          wchar_t c;
                          if(state==1)
                           {
                               c=(char)('\r');
                          }
                          else{
                                c=(char)('\n');
                              }
                          for(DWORD u=0;u<(fsize-nb);u++)
                          {
                              if(ftext[u]==c)
                              {
                                  output[track]=(char)('\r');
                                  track++;
                                  output[track]=(char)('\n');
                                  track++;
                              }
                              else{
                                    output[track]=ftext[u];
                                    track++;
                              }
                          }
                          GlobalFree(ftext);
                          ftext=output;
                          ftext[fsize]=(char)('\0');
                      }

                      HWND hedit=GetDlgItem(hwnd,IDM_TEXT);
                      SendMessage(hedit,WM_SETTEXT,0,(LPARAM)(char*)ftext);
                      SendMessage(hedit,EM_FMTLINES,TRUE,0);
                      SendMessage(hwnd,WM_SETTEXT,0,(LPARAM)fname);
                      SetDlgItemText(hwnd,IDM_FILENAME,fname);
                      UpdateWindow(hwnd);
                      SetFocus(hedit);
                      update_recent_file(hwnd,fname);
                      result=TRUE;
                      Setfile[0]=TRUE;
                      }
                     GlobalFree(ftext);
                    }
                }
                CloseHandle(hfile);
                if(!backup_file(fname,BACKUP))
                    {
                        Setfile[1]=FALSE;
                        EnableWindow(GetDlgItem(hwnd,IDCANCEL),FALSE);/**<Desactivation du button "Cancel"*/
                    }
                else{
                        Setfile[1]=TRUE;
                        EnableWindow(GetDlgItem(hwnd,IDCANCEL),TRUE);/**<Activation du button "Cancel"*/
                    }
        }
    return result;
}

DLL_EXPORT void set_bkcolor(HWND hwnd,COLORREF* bkcolor)
{
    InitCommonControls();
    COLORREF custom[16]={0};

    CHOOSECOLOR cc = {sizeof(CHOOSECOLOR)};

    cc.Flags = CC_RGBINIT | CC_FULLOPEN | CC_ANYCOLOR;
    cc.hwndOwner = hwnd;
    cc.lpTemplateName="TextEditor Color";
    cc.rgbResult = *bkcolor;
    cc.lpCustColors = custom;

    if(ChooseColor(&cc))
    {
        *bkcolor = cc.rgbResult;
    }
}

DLL_EXPORT void font_copy(CHAR* font, const char* value)
{
    int length=(int)strlen(value);
    int i;
    for(i=0;i<length;i++)
    {
        font[i]=value[i];
    }
    font[length]='\0';
}

/**Name: initialise
 *Objectif: redonne l'aspect initial de la fenetre
 *Entree: le HWND de la fenetre
          l'adresse des booleens setfile,change, qui permettent de savoir si un fichier est ouvert et si son contenu est modifie sans sauvegarde respectivement
          le  HWND du menu de l'application
 *Sorte: rien
 */
DLL_EXPORT void initialise(HWND hwnd, BOOL* change,BOOL* Setfile,HMENU* d_menu)
{
                    *change=FALSE;
                    Setfile[0]=FALSE;
                    Setfile[1]=FALSE;
                    backup_file(NULL,DEL_BACKUP);
                    SendMessage(hwnd,WM_SETTEXT,0,(LPARAM)(LPSTR)"TextEditor");
                    SetDlgItemText(hwnd,IDM_FILENAME,"***No input file***");
                    SetDlgItemText(hwnd,IDM_TEXT,"***Text Editor***");
                    SendMessage(GetDlgItem(hwnd,IDM_TEXT),EM_SETMODIFY,FALSE,0);
                    EnableWindow(GetDlgItem(hwnd,IDM_SAVE),FALSE);//Desactivation du button "save"
                    EnableWindow(GetDlgItem(hwnd,IDCANCEL),FALSE);//Desactivation du button "Cancel"
                    ModifyMenu(*d_menu,IDM_SAVE,MF_BYCOMMAND|MF_DISABLED,IDM_SAVE,"Save Changes\tCtrl+S"); //Desactivation de "save changes" dans les menus
                    ModifyMenu(*d_menu,IDM_SAVE_AS,MF_BYCOMMAND|MF_DISABLED,IDM_SAVE_AS,"Save As\tCtrl+Shift+S"); //Desactivation de "save As" dans les menus

}

DLL_EXPORT void hide_state_bar(HWND hwnd,BOOL* state)
{
    HMENU menu=GetMenu(hwnd);
    ModifyMenuA(menu,IDM_S_STATE_BAR,MF_BYCOMMAND|MF_ENABLED,IDM_S_STATE_BAR,"&Show state bar");
    RECT rect;
    GetClientRect(hwnd,&rect);
    HWND temp=GetDlgItem(hwnd,IDM_TEXT);
    MoveWindow(temp,5,50,rect.right-rect.left-6,rect.bottom-rect.top-55,1);
    ShowWindow(temp,SW_SHOWNORMAL);
    temp=GetDlgItem(hwnd,ID_STATUSBAR);
    ShowWindow(temp,SW_HIDE);
    *state=FALSE;
}

DLL_EXPORT void show_state_bar(HWND hwnd,BOOL* state)
{
    HMENU menu=GetMenu(hwnd);
    ModifyMenuA(menu,IDM_S_STATE_BAR,MF_BYCOMMAND|MF_ENABLED,IDM_S_STATE_BAR,"&Hide state bar");
    RECT rect,rc;
    HWND temp=GetDlgItem(hwnd,ID_STATUSBAR);
    SendMessage(temp, WM_SIZE, 0, 0);
    ShowWindow(temp,SW_SHOWNORMAL);
    GetClientRect(temp,&rc);
    GetClientRect(hwnd,&rect);
    temp=GetDlgItem(hwnd,IDM_TEXT);
    MoveWindow(temp,5,50,rect.right-rect.left-6,rect.bottom-rect.top-52-rc.bottom+rc.top,1);
    ShowWindow(temp,SW_SHOWNORMAL);

    *state=TRUE;
}

/**Name: update_recent_file
 *Objectif: ajoute(s'il n'y ait pas deja) le path pris en argument a la list box et le defini comme choix (recent file)
 *Entree: le HWND de la fenetre
          le path en question
 *Sortie: rien
 */
DLL_EXPORT void update_recent_file(HWND hwnd_window,const char* path)
{
    HWND hwnd=GetDlgItem(hwnd_window,IDM_CAPTION);
    int nb=SendMessage(hwnd,CB_GETCOUNT,0,0);/**<Nombre de D'entree dans la liste box recent file*/
    if (nb==CB_ERR)/**<Cas liste vide*/
    {
        SendMessage(hwnd,CB_ADDSTRING,0,(LPARAM)path);
        SendMessage(hwnd,CB_SETCURSEL,0,0);
    }
    else{
            int i=0;
            int size=0;
            int test=0;
            char tmp[MAX_PATH];
            while( (i<nb) && (test==0))/**<Parcours la liste en verifiant si le path n'y ait pas present*/
            {
                size=SendMessage(hwnd,CB_GETLBTEXT,(WPARAM)i,(LPARAM)tmp);
                if(size!=CB_ERR)
                {
                   if(strcmp(tmp,path)==0)test=1;
                   i++;
                }
            }
            if(test==0)/**<S'il n'y ait pas present*/
            {
                SendMessage(hwnd,CB_ADDSTRING,0,(LPARAM)path);
                SendMessage(hwnd,CB_SETCURSEL,nb,0);
            }
            else{ SendMessage(hwnd,CB_SETCURSEL,i-1,0);}//si on le trouve on le defini comme choix
        }
}

DLL_EXPORT int String_to_Int(const LPSTR ctext)
{
    int uint=0,i,t=1;
    int ulen=strlen(ctext);
    for(i=0;i<ulen;i++)
    {
       uint+= ((int)ctext[(ulen-1-i)]-48)*t;
       t=t*10;
    }
    return uint;
}

/**Name: report_error (incomplet)
 * Objectif: signaler une erreur survenue par email a l'addresse script@gmail.com
 * Entree: le HWND de la fenetre
           le code de l'errerur (result) obtenu en appelant GetLastError
           un flag qui determine la nature du commpn control.(see manage_error function)
 *Sortie: Nothing. L'application de messagerie de windows s'ouvrira et il suffira de cliquer sur "send" pour signaler l'erreur
 */
DLL_EXPORT void report_error(HWND hwnd,DWORD result,int flag,char* message_error)
{
    LPSTR email=NULL;
    if(MessageBox(hwnd,"Do you want to report this error?","Note",MB_YESNO|MB_ICONQUESTION)==IDYES)
    {
    char part1[]="mailto:0.my.script.1@gmail.com?subject=TextEditor:Report error&body=";
    char part2[]="&attachment=script.exe";
    email=(LPSTR)LocalAlloc(GPTR,sizeof(part1)+sizeof(message_error)+sizeof(part2)+1);
   if(email)
   {
        strcpy(email,part1);
        strcat(email,message_error);
        strcat(email,part2);
        ShellExecute(hwnd,"open",email,NULL,NULL,SW_SHOWNORMAL);
        LocalFree(email);
   }
   else{MessageBox(hwnd,"Some error occurs when we attempt to send report to 0.my.script.1@gmail.com.\n \tWe are sorry! \n\tPlease do it manually","MS-TextEditor[error]",MB_OK|MB_ICONERROR);}

    }
}

/**Name:manage_error(find/replace text,Dialog Font,Dialog Color non gere)
 * Objectif: gestion de toutes les erreurs des common control:Getopenfilename,Savefilename,find/replace text,Dialog Font,Dialog Color
 * Entree: le HWND de la fenetre
           le code de l'errerur (result) obtenu en appelant GetLastError
           un flag qui determine la nature du commpn control.
           Liste des flags: C_OPENFILENAME,C_SAVEFILENAME, C_FINDTEXT,C_REPLACETEXT
 * Sortie: rien.Une Dialog box sera affichee avec les info sur l'erreur survenue avec la possibilite de report cette erreur(gerer par la fonction report_error)
 */
DLL_EXPORT void manage_error(HWND hwnd,DWORD result,int flag)
{
    int test=0;
    switch(flag)
    {
    case C_OPENFILENAME:
    case C_SAVEFILENAME:
        switch(result)
        {
           case FNERR_BUFFERTOOSMALL:
             if(MessageBox(hwnd,TEXT("The buffer pointed to by the lpstrFile member of the OPENFILENAME structure is too small for the filename specified by the user..\nDo you want to report error ?"),TEXT("Error"),MB_YESNO|MB_ICONERROR)==IDYES)
             {
               report_error(hwnd,result,flag,NULL);
             }
             test=1;
             break;
           case FNERR_INVALIDFILENAME:
             MessageBox(hwnd,TEXT("A filename is invalid."),TEXT("Error"),MB_OK|MB_ICONERROR);
             test=1;
             break;

           case FNERR_SUBCLASSFAILURE:
             MessageBox(hwnd,TEXT("Sufficient memory was not available."),TEXT("Error"),MB_OK|MB_ICONERROR);
             test=1;
             break;
           default:
             break;
        }
        break;
     case C_FINDTEXT:
     case C_REPLACETEXT:
        if(result==FRERR_BUFFERLENGTHZERO)
        {
             if(MessageBox(hwnd,TEXT("A member of the FINDREPLACE structure points to an invalid buffer.Do you want to report error ?"),TEXT("Error"),MB_YESNO|MB_ICONERROR)==IDYES)
             {
                  report_error(hwnd,result,flag,NULL);
             }
             test=1;
        }
        break;
     default:
        break;
    }

    if(test==0)
    {

     switch(result)
     {
        case CDERR_DIALOGFAILURE:
            MessageBox(hwnd,TEXT("Invalid window handle.\nTry again later"),TEXT("Error"),MB_OK|MB_ICONERROR);
            break;
        case CDERR_FINDRESFAILURE:
            MessageBox(hwnd,TEXT("Application failed to find a specified resource.\nTry again later"),TEXT("Error"),MB_OK|MB_ICONERROR);
            break;
        case CDERR_INITIALIZATION:
        case CDERR_MEMALLOCFAILURE:
        case CDERR_MEMLOCKFAILURE:
            MessageBox(hwnd,TEXT("sufficient memory is not available.\n"),TEXT("Error"),MB_OK|MB_ICONERROR);
            break;
        case CDERR_LOADRESFAILURE:
        case CDERR_LOADSTRFAILURE:
        case CDERR_LOCKRESFAILURE:
             MessageBox(hwnd,TEXT("Loading of specified resource failed.\n"),TEXT("Error"),MB_OK|MB_ICONERROR);
            break;
        case CDERR_NOHINSTANCE:
            if(MessageBox(hwnd,TEXT("Texteditor failed to provide a corresponding instance handle.\nDo you want to report error ?"),TEXT("Error"),MB_YESNO|MB_ICONERROR)==IDYES)
             {
                  report_error(hwnd,result,flag,NULL);
             }
             break;
        case CDERR_NOHOOK:
            if(MessageBox(hwnd,TEXT("TextEditor failed to provide a pointer to a corresponding hook procedure.\nDo you want to report error ?"),TEXT("Error"),MB_YESNO|MB_ICONERROR)==IDYES)
             {
               report_error(hwnd,result,flag,NULL);
             }
             break;
        case CDERR_NOTEMPLATE:
            if(MessageBox(hwnd,TEXT("TextEditor failed to provide a pointer to a corresponding template.\nDo you want to report error ?"),TEXT("Error"),MB_YESNO|MB_ICONERROR)==IDYES)
             {
               report_error(hwnd,result,flag,NULL);
             }
             break;
        case CDERR_REGISTERMSGFAIL:
            if(MessageBox(hwnd,TEXT("TextEditor:The RegisterWindowMessage function returned an error code .\nDo you want to report error ?"),TEXT("Error"),MB_YESNO|MB_ICONERROR)==IDYES)
             {
               report_error(hwnd,result,flag,NULL);
             }
             break;
        case CDERR_STRUCTSIZE:
            if(MessageBox(hwnd,TEXT("TextEditor:The RegisterWindowMessage function returned an error code .\nDo you want to report error ?"),TEXT("Error"),MB_YESNO|MB_ICONERROR)==IDYES)
             {
               report_error(hwnd,result,flag,NULL);
             }
             break;
        default:
            MessageBox(hwnd,TEXT("Unknown error was occured.\nTry again later"),TEXT("Error"),MB_OK|MB_ICONERROR);
            break;
      }
    }
}

/** Name:fileExist
 * Objectif:verifie si un fichier existe
 * Entree: le chemin du fichier(absolue ou relatif)
 *
 * Sortie: TRUE s'il existe et FALSE sinon
 */
DLL_EXPORT BOOL fileExist(const TCHAR* file)
{
  /*WIN32_FIND_DATA FindFileData;
  HANDLE handle=FindFirstFile(file,&FindFileData);
  BOOL result = handle != INVALID_HANDLE_VALUE;
  if(result)
  {
      FindClose(handle);
  }*/
  FILE* h_file=NULL;
  h_file=fopen(file,"r");
  BOOL result=FALSE;
  if(h_file!=NULL)
  {
      result=TRUE;
  }
  fclose(h_file);
  return result;
}

/**Name: read_error
 * Objectif:  Gestion des erreurs lors de la lecture d'un fichier
 * Entree: le HWND de la fenetre et le code de l'erreur obtenu en executant l fonction GetLastError
 * Sortie: rien. Affiche une boite de dialogue donnant les info sur l'erreur survenue
 */
DLL_EXPORT void read_error(HWND hwnd,DWORD dwError)
{
    switch (dwError)
    {
        case ERROR_HANDLE_EOF:
        {
            MessageBox(hwnd,TEXT("we're reached the end of the file during the call to ReadFile"),TEXT("Error"),MB_OK|MB_ICONERROR);
            break;
        }
        case ERROR_IO_PENDING:
        {
            MessageBox(hwnd,TEXT("File is openned by another program"),TEXT("Error"),MB_OK|MB_ICONERROR);
            break;
        }
        default:
            MessageBox(hwnd,TEXT("Uknown error was occured"),TEXT("Error"),MB_OK|MB_ICONERROR);
            break;
    }
}

DLL_EXPORT BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            // attach to process
            // return FALSE to fail DLL load
            break;

        case DLL_PROCESS_DETACH:
            // detach from process
            break;

        case DLL_THREAD_ATTACH:
            // attach to thread
            break;

        case DLL_THREAD_DETACH:
            // detach from thread
            break;
    }
    return TRUE; // succesful
}

#ifdef __cplusplus
}
#endif
