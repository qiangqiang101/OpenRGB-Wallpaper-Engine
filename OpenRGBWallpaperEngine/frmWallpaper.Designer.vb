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
        Me.components = New System.ComponentModel.Container()
        Dim resources As System.ComponentModel.ComponentResourceManager = New System.ComponentModel.ComponentResourceManager(GetType(frmWallpaper))
        Me.tmUpdate = New System.Windows.Forms.Timer(Me.components)
        Me.SuspendLayout()
        '
        'tmUpdate
        '
        Me.tmUpdate.Enabled = True
        Me.tmUpdate.Interval = 10
        '
        'frmWallpaper
        '
        Me.AutoScaleMode = System.Windows.Forms.AutoScaleMode.None
        Me.BackColor = System.Drawing.Color.Black
        Me.ClientSize = New System.Drawing.Size(50, 50)
        Me.ControlBox = False
        Me.DoubleBuffered = True
        Me.ForeColor = System.Drawing.Color.White
        Me.FormBorderStyle = System.Windows.Forms.FormBorderStyle.None
        Me.Icon = CType(resources.GetObject("$this.Icon"), System.Drawing.Icon)
        Me.MaximizeBox = False
        Me.MinimizeBox = False
        Me.Name = "frmWallpaper"
        Me.ShowIcon = False
        Me.ShowInTaskbar = False
        Me.StartPosition = System.Windows.Forms.FormStartPosition.Manual
        Me.Text = "OpenRGB Wallpaper"
        Me.ResumeLayout(False)

    End Sub

    Friend WithEvents tmUpdate As Timer
End Class
