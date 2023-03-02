Imports System.Drawing.Drawing2D
Imports System.Drawing.Imaging
Imports System.IO
Imports System.Runtime.CompilerServices
Imports Color = System.Drawing.Color
Imports WindowsDisplayAPI.DisplayConfig
Imports Newtonsoft.Json.Linq

Module Utils

    Public SmoothingMode As SmoothingMode = SmoothingMode.Default
    Public CompositingQuality As CompositingQuality = CompositingQuality.Default
    Public InterpolationMode As InterpolationMode = InterpolationMode.Default
    Public PixelOffsetMode As PixelOffsetMode = PixelOffsetMode.Default
    Public LEDShape As LEDShape = LEDShape.Rectangle
    Public LEDPadding As Single = 0
    Public TimerIntervals As Integer = 30
    Public RoundedRectangleCornerRadius As Integer = 10

    'SDK Client
    Public IPAddress As String = "127.0.0.1"
    Public Port As Integer = 6742
    Public DeviceName As String = "Wallpaper1"

    'Display
    Public BackgroundImage As String = Nothing
    Public ImageFit As ImageFit = ImageFit.Fill
    Public BackgroundColor As String = ColorTranslator.ToHtml(Color.Black)

    Public Sub UpdateWEConfigValues()
        Try
            Dim config As String = WallpaperEngineConfig()
            Dim username As String = SystemInformation.UserName
            Dim mypath As String = Application.ExecutablePath.Replace("\", "/")
            Dim display As String = frmWallpaper.ScreenDevicePath
            Dim json = JObject.Parse(File.ReadAllText(config))

            BackgroundImage = json(username)("wproperties")(mypath)(display)("coverImage")
        Catch ex As Exception
            Logger.Log($"{ex.Message} {ex.StackTrace}")
        End Try
    End Sub

    <Extension>
    Public Sub DrawRoundedRectangle(graphics As Graphics, pen As Pen, bounds As Rectangle, radius As Integer)
        If graphics Is Nothing Then
            Throw New ArgumentNullException("graphics")
        End If
        If pen Is Nothing Then
            Throw New ArgumentNullException("prush")
        End If

        Using path As GraphicsPath = RoundedRect(bounds, radius)
            graphics.DrawPath(pen, path)
        End Using
    End Sub

    <Extension>
    Public Sub FillRoundedRectangle(graphics As Graphics, brush As Brush, bounds As Rectangle, radius As Integer)
        If graphics Is Nothing Then
            Throw New ArgumentNullException("graphics")
        End If
        If brush Is Nothing Then
            Throw New ArgumentNullException("brush")
        End If

        Using path As GraphicsPath = RoundedRect(bounds, radius)
            graphics.FillPath(brush, path)
        End Using
    End Sub

    Private Function RoundedRect(bounds As Rectangle, radius As Integer) As GraphicsPath
        Dim diameter As Integer = radius * 2
        Dim size As Size = New Size(diameter, diameter)
        Dim arc As Rectangle = New Rectangle(bounds.Location, size)
        Dim path As GraphicsPath = New GraphicsPath

        If (radius = 0) Then
            path.AddRectangle(bounds)
            Return path
        End If

        'top left arc
        path.AddArc(arc, 180, 90)

        'top right arc
        arc.X = bounds.Right - diameter
        path.AddArc(arc, 270, 90)

        'bottom right arc
        arc.Y = bounds.Bottom - diameter
        path.AddArc(arc, 0, 90)

        'bottom left arc
        arc.X = bounds.Left
        path.AddArc(arc, 90, 90)

        path.CloseFigure()
        Return path
    End Function

    <Extension>
    Public Function Base64ToImage(Image As String) As Image
        Try
            If Image = Nothing Then
                Return Nothing
            Else
                Dim b64 As String = Image.Replace(" ", "+")
                Dim bite() As Byte = Convert.FromBase64String(b64)
                Dim stream As New MemoryStream(bite)
                Return Drawing.Image.FromStream(stream)
            End If
        Catch ex As Exception
            Return Nothing
        End Try
    End Function

    <Extension>
    Public Function ImageToBase64(img As Image, Optional forceFormat As ImageFormat = Nothing, Optional formatting As Base64FormattingOptions = Base64FormattingOptions.InsertLineBreaks) As String
        Try
            If img IsNot Nothing Then
                If forceFormat Is Nothing Then forceFormat = img.RawFormat
                Dim stream As New MemoryStream
                img.Save(stream, forceFormat)
                Return Convert.ToBase64String(stream.ToArray, formatting)
            Else
                Return Nothing
            End If
        Catch ex As Exception
            Return Nothing
        End Try
    End Function

    <Extension>
    Public Function ResizeImage(ByVal image As Image, ByVal size As Size, Optional ByVal preserveAspectRatio As Boolean = True) As Image
        Dim newWidth As Integer
        Dim newHeight As Integer
        If preserveAspectRatio Then
            Dim originalWidth As Integer = image.Width
            Dim originalHeight As Integer = image.Height
            Dim percentWidth As Single = CSng(size.Width) / CSng(originalWidth)
            Dim percentHeight As Single = CSng(size.Height) / CSng(originalHeight)
            Dim percent As Single = If(percentHeight < percentWidth,
                    percentHeight, percentWidth)
            newWidth = CInt(originalWidth * percent)
            newHeight = CInt(originalHeight * percent)
        Else
            newWidth = size.Width
            newHeight = size.Height
        End If
        Dim newImage As Image = New Bitmap(newWidth, newHeight)
        Using graphicsHandle As Graphics = Graphics.FromImage(newImage)
            graphicsHandle.InterpolationMode = InterpolationMode.HighQualityBicubic
            graphicsHandle.DrawImage(image, 0, 0, newWidth, newHeight)
        End Using
        Return newImage
    End Function

    <Extension>
    Public Function ToColor(modelcolor As OpenRGB.NET.Color) As Color
        Return Color.FromArgb(modelcolor.R, modelcolor.G, modelcolor.B)
    End Function

    Public Function WallpaperEngineConfig() As String
        Dim wallpaper32 As Process = Process.GetProcessesByName("wallpaper32").FirstOrDefault
        Dim wallpaper64 As Process = Process.GetProcessesByName("wallpaper64").FirstOrDefault

        Try
            If wallpaper32 Is Nothing Then
                Return $"{Path.GetDirectoryName(wallpaper64.MainModule.FileName)}\config.json"
            Else
                Return $"{Path.GetDirectoryName(wallpaper32.MainModule.FileName)}\config.json"
            End If
        Catch ex As Exception
            Logger.Log($"{ex.Message} {ex.StackTrace}")
            Return "error"
        End Try
    End Function

    <Extension>
    Public Function ScreenDevicePath(form As Form) As String
        Dim currScreen As Screen = Screen.FromControl(form)
        Dim currDisplay = PathDisplayTarget.GetDisplayTargets.Where(Function(x) x.ToDisplayDevice.DisplayName = currScreen.DeviceName).FirstOrDefault
        Return currDisplay.DevicePath.Replace("\", "/")
    End Function

End Module

Public Enum ImageFit
    None
    Fill
    Fit
    Stretch
    Center
End Enum

Public Enum LEDShape
    Rectangle
    RoundedRectangle
    Sphere
End Enum