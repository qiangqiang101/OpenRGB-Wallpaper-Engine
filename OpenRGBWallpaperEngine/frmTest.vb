Imports Newtonsoft.Json
Imports Newtonsoft.Json.Linq
Imports WindowsDisplayAPI.DisplayConfig

Public Class frmTest
    Private Sub frmTest_Load(sender As Object, e As EventArgs) Handles MyBase.Load
        Dim config As String = WallpaperEngineConfig()
        Dim username As String = SystemInformation.UserName
        Dim mypath As String = "G:/Program Files (x86)/Steam/steamapps/common/wallpaper_engine/projects/myprojects/openrgbwallpaper/OpenRGBWallpaperEngine.exe" 'Application.ExecutablePath
        Dim display As String = frmWallpaper.ScreenDevicePath
        Dim json As JObject = JObject.Parse(IO.File.ReadAllText(config))

        ListBox1.Items.Add(json(username)("wproperties")(mypath)(display)("coverImage"))
    End Sub
End Class