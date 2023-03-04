<Global.Microsoft.VisualBasic.CompilerServices.DesignerGenerated()>
Partial Class frmWallpaper
    Inherits System.Windows.Forms.Form

    'Form overrides dispose to clean up the component list.
    <System.Diagnostics.DebuggerNonUserCode()>
    Protected Overrides Sub Dispose(ByVal disposing As Boolean)
        Try
            If disposing AndAlso components IsNot Nothing Then
                components.Dispose()
            End If
        Finally
            MyBase.Dispose(disposing)
        End Try
    End Sub

    'Required by the Windows Form Designer
    Private components As System.ComponentModel.IContainer

    'NOTE: The following procedure is required by the Windows Form Designer
    'It can be modified using the Windows Form Designer.  
    'Do not modify it using the code editor.
    <System.Diagnostics.DebuggerStepThrough()>
    Private Sub InitializeComponent()
        components = New ComponentModel.Container()
        Dim resources As ComponentModel.ComponentResourceManager = New ComponentModel.ComponentResourceManager(GetType(frmWallpaper))
        tmUpdate = New Timer(components)
        tmCheckOpenRGB = New Timer(components)
        tmConfig = New Timer(components)
        SuspendLayout()
        ' 
        ' tmUpdate
        ' 
        tmUpdate.Enabled = True
        tmUpdate.Interval = 10
        ' 
        ' tmCheckOpenRGB
        ' 
        tmCheckOpenRGB.Interval = 30000
        ' 
        ' tmConfig
        ' 
        tmConfig.Enabled = True
        tmConfig.Interval = 10000
        ' 
        ' frmWallpaper
        ' 
        AutoScaleMode = AutoScaleMode.None
        BackColor = Drawing.Color.Black
        ClientSize = New Size(192, 108)
        ControlBox = False
        DoubleBuffered = True
        ForeColor = Drawing.Color.White
        FormBorderStyle = FormBorderStyle.None
        Icon = CType(resources.GetObject("$this.Icon"), Icon)
        MaximizeBox = False
        MinimizeBox = False
        Name = "frmWallpaper"
        ShowIcon = False
        ShowInTaskbar = False
        StartPosition = FormStartPosition.Manual
        Text = "OpenRGB Wallpaper"
        ResumeLayout(False)
    End Sub

    Friend WithEvents tmUpdate As Timer
    Friend WithEvents tmCheckOpenRGB As Timer
    Friend WithEvents tmConfig As Timer
End Class
