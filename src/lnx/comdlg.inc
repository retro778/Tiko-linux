'    tiko editor (Linux port)
'    Copyright (C) 2016-2026 Paul Squires, PlanetSquires Software
'    Licensed under the GNU GPLv3 or later.
'
' Minimal emulation of the COM objects tiko uses for the Windows file dialogs
' (IFileOpenDialog, IFileSaveDialog, IShellItem, IShellItemArray) on top of
' GtkFileChooserDialog, so that the upstream dialog code compiles unchanged.

#pragma once

type CLSID as GUID
type IID as GUID
type SIGDN as long
type FILEOPENDIALOGOPTIONS as DWORD

#ifndef SIGDN_NORMALDISPLAY
#define SIGDN_NORMALDISPLAY        &h00000000
#endif
#ifndef SIGDN_FILESYSPATH
#define SIGDN_FILESYSPATH          &h80058000
#endif
#ifndef FOS_OVERWRITEPROMPT
#define FOS_OVERWRITEPROMPT        &h2
#endif
#ifndef FOS_PICKFOLDERS
#define FOS_PICKFOLDERS            &h20
#endif
#ifndef FOS_FORCEFILESYSTEM
#define FOS_FORCEFILESYSTEM        &h40
#endif
#ifndef FOS_ALLOWMULTISELECT
#define FOS_ALLOWMULTISELECT       &h200
#endif
#ifndef FOS_PATHMUSTEXIST
#define FOS_PATHMUSTEXIST          &h800
#endif
#ifndef CLSCTX_INPROC_SERVER
#define CLSCTX_INPROC_SERVER       &h1
#endif
#ifndef HRESULT_ERROR_CANCELLED
#define HRESULT_ERROR_CANCELLED    cast(HRESULT, &h800704C7)
#endif
#define SUCCEEDED(hr)              (cast(HRESULT, (hr)) >= 0)
#define FAILED(hr)                 (cast(HRESULT, (hr)) < 0)

type COMDLG_FILTERSPEC
    pszName as LPCWSTR
    pszSpec as LPCWSTR
end type

dim shared as CLSID CLSID_FileOpenDialog = ( &hDC1C5A9C, &hE88A, &h4DDE, {&hA5, &hA1, &h60, &hF8, &h2A, &h20, &hAE, &hF7} )
dim shared as CLSID CLSID_FileSaveDialog = ( &hC0B4E2F3, &hBA21, &h4773, {&h8D, &hBA, &h33, &h5E, &hC9, &h46, &hEB, &h8B} )
dim shared as IID   IID_IFileOpenDialog  = ( &hD57C7288, &hD4AD, &h4768, {&hBE, &h02, &h9D, &h96, &h95, &h32, &hD9, &h60} )
dim shared as IID   IID_IFileSaveDialog  = ( &h84BCCD23, &h5FDE, &h4CDB, {&hAE, &hA4, &hAF, &h64, &hB8, &h3D, &h78, &hAB} )
dim shared as IID   IID_IShellItem       = ( &h43826D1E, &hE718, &h42EE, {&hBC, &h55, &hA1, &hE2, &h61, &hC3, &h7B, &hFE} )

type IShellItem_ as IShellItem
type IShellItemArray_ as IShellItemArray
type IFileDialog_ as IFileDialog

' ---- IShellItem ----
type IShellItemVtbl
    Release        as function( byval this_ as IShellItem_ ptr ) as ULONG
    GetDisplayName as function( byval this_ as IShellItem_ ptr, byval sigdnName as SIGDN, byval ppszName as wstring ptr ptr ) as HRESULT
end type
type IShellItem
    lpVtbl as IShellItemVtbl ptr
    path   as DWSTRING
end type

private function LnxShellItem_Release( byval this_ as IShellItem ptr ) as ULONG
    if this_ then delete this_
    return 0
end function

private function LnxShellItem_GetDisplayName( byval this_ as IShellItem ptr, byval sigdnName as SIGDN, byval ppszName as wstring ptr ptr ) as HRESULT
    if this_ = 0 orelse ppszName = 0 then return E_INVALIDARG
    dim as DWSTRING s = this_->path
    if sigdnName = SIGDN_NORMALDISPLAY then s = AfxStrPathname( "NAMEX", s )
    ' caller frees with CoTaskMemFree
    dim as wstring ptr p = allocate( (len(s) + 1) * sizeof(wstring) )
    memcpy( p, s.vptr, (len(s) + 1) * sizeof(wstring) )
    *ppszName = p
    return S_OK
end function

dim shared as IShellItemVtbl gLnxShellItemVtbl = ( @LnxShellItem_Release, @LnxShellItem_GetDisplayName )

private function LnxNewShellItem( byref wszPath as const wstring ) as IShellItem ptr
    dim as IShellItem ptr p = new IShellItem
    p->lpVtbl = @gLnxShellItemVtbl
    p->path = NativePath(wszPath)
    return p
end function

private function SHCreateItemFromParsingName( byval pszPath as LPCWSTR, byval pbc as any ptr, byval riid as const IID ptr, byval ppv as any ptr ptr ) as HRESULT
    if ppv = 0 then return E_INVALIDARG
    *ppv = 0
    if pszPath = 0 then return E_INVALIDARG
    if AfxFileExists(*pszPath) = false then return E_FAIL
    *ppv = LnxNewShellItem( *pszPath )
    return S_OK
end function

private sub CoTaskMemFree( byval pv as any ptr )
    if pv then deallocate( pv )
end sub

' ---- IShellItemArray ----
type IShellItemArrayVtbl
    Release   as function( byval this_ as IShellItemArray_ ptr ) as ULONG
    GetCount  as function( byval this_ as IShellItemArray_ ptr, byval pdwNumItems as DWORD ptr ) as HRESULT
    GetItemAt as function( byval this_ as IShellItemArray_ ptr, byval dwIndex as DWORD, byval ppsi as IShellItem ptr ptr ) as HRESULT
end type
type IShellItemArray
    lpVtbl as IShellItemArrayVtbl ptr
    paths(any) as DWSTRING
    count as long
end type

private function LnxShellItemArray_Release( byval this_ as IShellItemArray ptr ) as ULONG
    if this_ then delete this_
    return 0
end function
private function LnxShellItemArray_GetCount( byval this_ as IShellItemArray ptr, byval pdwNumItems as DWORD ptr ) as HRESULT
    if this_ = 0 orelse pdwNumItems = 0 then return E_INVALIDARG
    *pdwNumItems = this_->count
    return S_OK
end function
private function LnxShellItemArray_GetItemAt( byval this_ as IShellItemArray ptr, byval dwIndex as DWORD, byval ppsi as IShellItem ptr ptr ) as HRESULT
    if this_ = 0 orelse ppsi = 0 orelse dwIndex >= this_->count then return E_INVALIDARG
    *ppsi = LnxNewShellItem( this_->paths(dwIndex) )
    return S_OK
end function
dim shared as IShellItemArrayVtbl gLnxShellItemArrayVtbl = ( @LnxShellItemArray_Release, @LnxShellItemArray_GetCount, @LnxShellItemArray_GetItemAt )

' ---- IFileDialog (open and save) ----
type IFileDialogVtbl
    Release             as function( byval this_ as IFileDialog_ ptr ) as ULONG
    Show                as function( byval this_ as IFileDialog_ ptr, byval hwndOwner as HWND ) as HRESULT
    SetFileTypes        as function( byval this_ as IFileDialog_ ptr, byval cFileTypes as UINT, byval rgFilterSpec as const COMDLG_FILTERSPEC ptr ) as HRESULT
    SetFileTypeIndex    as function( byval this_ as IFileDialog_ ptr, byval iFileType as UINT ) as HRESULT
    SetOptions          as function( byval this_ as IFileDialog_ ptr, byval fos as FILEOPENDIALOGOPTIONS ) as HRESULT
    GetOptions          as function( byval this_ as IFileDialog_ ptr, byval pfos as FILEOPENDIALOGOPTIONS ptr ) as HRESULT
    SetFolder           as function( byval this_ as IFileDialog_ ptr, byval psi as IShellItem ptr ) as HRESULT
    SetFileName         as function( byval this_ as IFileDialog_ ptr, byval pszName as LPCWSTR ) as HRESULT
    SetTitle            as function( byval this_ as IFileDialog_ ptr, byval pszTitle as LPCWSTR ) as HRESULT
    SetDefaultExtension as function( byval this_ as IFileDialog_ ptr, byval pszDefaultExtension as LPCWSTR ) as HRESULT
    GetResult           as function( byval this_ as IFileDialog_ ptr, byval ppsi as IShellItem ptr ptr ) as HRESULT
    GetResults          as function( byval this_ as IFileDialog_ ptr, byval ppenum as IShellItemArray ptr ptr ) as HRESULT
end type

type IFileDialog
    lpVtbl      as IFileDialogVtbl ptr
    bSave       as boolean
    title       as DWSTRING
    folder      as DWSTRING
    fileName    as DWSTRING
    defExt      as DWSTRING
    options     as DWORD
    filterNames(any) as DWSTRING
    filterSpecs(any) as DWSTRING
    filterCount as long
    filterIndex as long          ' 1 based
    results(any) as DWSTRING
    resultCount as long
end type
type IFileOpenDialog as IFileDialog
type IFileSaveDialog as IFileDialog

private function LnxFD_Release( byval this_ as IFileDialog ptr ) as ULONG
    if this_ then delete this_
    return 0
end function
private function LnxFD_SetFileTypes( byval this_ as IFileDialog ptr, byval cFileTypes as UINT, byval rg as const COMDLG_FILTERSPEC ptr ) as HRESULT
    this_->filterCount = cFileTypes
    if cFileTypes = 0 then return S_OK
    redim this_->filterNames(cFileTypes - 1)
    redim this_->filterSpecs(cFileTypes - 1)
    for i as long = 0 to cFileTypes - 1
        if rg[i].pszName then this_->filterNames(i) = *rg[i].pszName
        if rg[i].pszSpec then this_->filterSpecs(i) = *rg[i].pszSpec
    next
    return S_OK
end function
private function LnxFD_SetFileTypeIndex( byval this_ as IFileDialog ptr, byval iFileType as UINT ) as HRESULT
    this_->filterIndex = iFileType : return S_OK
end function
private function LnxFD_SetOptions( byval this_ as IFileDialog ptr, byval fos as FILEOPENDIALOGOPTIONS ) as HRESULT
    this_->options = fos : return S_OK
end function
private function LnxFD_GetOptions( byval this_ as IFileDialog ptr, byval pfos as FILEOPENDIALOGOPTIONS ptr ) as HRESULT
    if pfos then *pfos = this_->options
    return S_OK
end function
private function LnxFD_SetFolder( byval this_ as IFileDialog ptr, byval psi as IShellItem ptr ) as HRESULT
    if psi then this_->folder = psi->path
    return S_OK
end function
private function LnxFD_SetFileName( byval this_ as IFileDialog ptr, byval pszName as LPCWSTR ) as HRESULT
    if pszName then this_->fileName = *pszName
    return S_OK
end function
private function LnxFD_SetTitle( byval this_ as IFileDialog ptr, byval pszTitle as LPCWSTR ) as HRESULT
    if pszTitle then this_->title = *pszTitle
    return S_OK
end function
private function LnxFD_SetDefaultExtension( byval this_ as IFileDialog ptr, byval pszExt as LPCWSTR ) as HRESULT
    if pszExt then this_->defExt = *pszExt
    return S_OK
end function

private function LnxFD_Show( byval this_ as IFileDialog ptr, byval hwndOwner as HWND ) as HRESULT
    dim as HWND top = iif(hwndOwner, LnxToplevelOf(hwndOwner), GetActiveWindow())
    dim as string sTitle = this_->title.utf8
    dim as GtkFileChooserAction act = iif(this_->bSave, GTK_FILE_CHOOSER_ACTION_SAVE, GTK_FILE_CHOOSER_ACTION_OPEN)
    if (this_->options and FOS_PICKFOLDERS) then act = GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER
    dim as GtkWidget ptr dlg = gtk_file_chooser_dialog_new( LnxZ(sTitle), iif(top, GTK_WINDOW(top->widget), NULL), act, _
        "_Cancel", GTK_RESPONSE_CANCEL, iif(this_->bSave, @"_Save", @"_Open"), GTK_RESPONSE_ACCEPT, NULL )
    dim as GtkFileChooser ptr fc = GTK_FILE_CHOOSER(dlg)
    gtk_file_chooser_set_select_multiple( fc, iif((this_->options and FOS_ALLOWMULTISELECT), TRUE, FALSE) )
    gtk_file_chooser_set_do_overwrite_confirmation( fc, TRUE )
    gtk_file_chooser_set_local_only( fc, TRUE )

    ' filters ("*.bas;*.bi" -> patterns, also matching upper case extensions like Windows)
    dim as GtkFileFilter ptr filters(any)
    if this_->filterCount > 0 then redim filters(this_->filterCount - 1)
    for i as long = 0 to this_->filterCount - 1
        dim as GtkFileFilter ptr f = gtk_file_filter_new()
        dim as string nm = this_->filterNames(i).utf8 & " (" & this_->filterSpecs(i).utf8 & ")"
        gtk_file_filter_set_name( f, LnxZ(nm) )
        dim as long n = AfxStrParseCount( this_->filterSpecs(i), ";" )
        for k as long = 1 to n
            dim as string pat = trim( AfxStrParse(this_->filterSpecs(i), k, ";").utf8 )
            if pat = "*.*" then pat = "*"
            gtk_file_filter_add_pattern( f, LnxZ(pat) )
            if ucase(pat) <> pat then
                dim as string upat = ucase(pat)
                gtk_file_filter_add_pattern( f, LnxZ(upat) )
            end if
        next
        gtk_file_chooser_add_filter( fc, f )
        filters(i) = f
    next
    if this_->filterCount > 0 then
        dim as long idx = iif(this_->filterIndex >= 1 andalso this_->filterIndex <= this_->filterCount, this_->filterIndex - 1, 0)
        gtk_file_chooser_set_filter( fc, filters(idx) )
    end if

    if len(this_->folder) then
        dim as string fo = NativePath(this_->folder).utf8
        gtk_file_chooser_set_current_folder( fc, LnxZ(fo) )
    end if
    if this_->bSave andalso len(this_->fileName) then
        dim as DWSTRING fnm = NativePath(this_->fileName)
        if instr(fnm, "/") then
            dim as string fs = fnm.utf8
            if AfxFileExists(fnm) then gtk_file_chooser_set_filename( fc, LnxZ(fs) ) _
            else dim as string fb = AfxStrPathname("NAMEX", fnm).utf8 : gtk_file_chooser_set_current_name( fc, LnxZ(fb) )
        else
            dim as string fs = fnm.utf8
            gtk_file_chooser_set_current_name( fc, LnxZ(fs) )
        end if
    end if

    dim as long resp = gtk_dialog_run( GTK_DIALOG(dlg) )
    this_->resultCount = 0
    erase this_->results
    if resp = GTK_RESPONSE_ACCEPT then
        dim as GSList ptr lst = gtk_file_chooser_get_filenames( fc )
        dim as GSList ptr it = lst
        while it
            redim preserve this_->results(this_->resultCount)
            dim as DWSTRING r = *cast(zstring ptr, it->data)
            ' default extension for saved files
            if this_->bSave andalso len(this_->defExt) andalso len(AfxStrPathname("EXTN", r)) = 0 then
                r &= "." & this_->defExt
            end if
            this_->results(this_->resultCount) = r
            this_->resultCount += 1
            g_free( it->data )
            it = it->next
        wend
        g_slist_free( lst )
    end if
    gtk_widget_destroy( dlg )
    while gtk_events_pending() : gtk_main_iteration_do( FALSE ) : wend
    return iif( this_->resultCount > 0, S_OK, HRESULT_ERROR_CANCELLED )
end function

private function LnxFD_GetResult( byval this_ as IFileDialog ptr, byval ppsi as IShellItem ptr ptr ) as HRESULT
    if ppsi = 0 then return E_INVALIDARG
    *ppsi = 0
    if this_->resultCount = 0 then return E_FAIL
    *ppsi = LnxNewShellItem( this_->results(0) )
    return S_OK
end function

private function LnxFD_GetResults( byval this_ as IFileDialog ptr, byval ppenum as IShellItemArray ptr ptr ) as HRESULT
    if ppenum = 0 then return E_INVALIDARG
    *ppenum = 0
    if this_->resultCount = 0 then return E_FAIL
    dim as IShellItemArray ptr a = new IShellItemArray
    a->lpVtbl = @gLnxShellItemArrayVtbl
    redim a->paths(this_->resultCount - 1)
    for i as long = 0 to this_->resultCount - 1 : a->paths(i) = this_->results(i) : next
    a->count = this_->resultCount
    *ppenum = a
    return S_OK
end function

dim shared as IFileDialogVtbl gLnxFileDialogVtbl = ( @LnxFD_Release, @LnxFD_Show, @LnxFD_SetFileTypes, @LnxFD_SetFileTypeIndex, _
    @LnxFD_SetOptions, @LnxFD_GetOptions, @LnxFD_SetFolder, @LnxFD_SetFileName, @LnxFD_SetTitle, @LnxFD_SetDefaultExtension, _
    @LnxFD_GetResult, @LnxFD_GetResults )

private function CoCreateInstance( byval rclsid as const CLSID ptr, byval pUnkOuter as any ptr, byval dwClsContext as DWORD, _
                                   byval riid as const IID ptr, byval ppv as any ptr ptr ) as HRESULT
    if ppv = 0 orelse rclsid = 0 then return E_INVALIDARG
    *ppv = 0
    if rclsid->Data1 = CLSID_FileOpenDialog.Data1 orelse rclsid->Data1 = CLSID_FileSaveDialog.Data1 then
        dim as IFileDialog ptr d = new IFileDialog
        d->lpVtbl = @gLnxFileDialogVtbl
        d->bSave = (rclsid->Data1 = CLSID_FileSaveDialog.Data1)
        d->options = FOS_FILEMUSTEXIST
        *ppv = d
        return S_OK
    end if
    return E_FAIL
end function

private function CoInitialize( byval pvReserved as any ptr ) as HRESULT
    return S_OK
end function
private sub CoUninitialize()
end sub

' Folder picker
private function AfxBrowseForFolder( byval hwnd as HWND, byref wszTitle as const wstring = "", _
                                     byref wszStartFolder as const wstring = "", byval nFlags as long = 0 ) as DWSTRING
    dim as IFileDialog ptr d
    CoCreateInstance( @CLSID_FileOpenDialog, NULL, 0, @IID_IFileOpenDialog, @d )
    d->options = FOS_PICKFOLDERS
    d->title = wszTitle
    d->folder = wszStartFolder
    dim as DWSTRING result
    if LnxFD_Show( d, hwnd ) = S_OK then result = d->results(0)
    LnxFD_Release( d )
    return result
end function
