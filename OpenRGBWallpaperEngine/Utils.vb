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
    Public CpuUsagePauseValue As Integer = 60

    Public IPAddress As String = "127.0.0.1"
    Public Port As Integer = 6742
    Public DeviceName As String = "Wallpaper1"

    Public BackgroundImage As String = Nothing
    Public ImageFit As ImageFit = ImageFit.Fill
    Public BackgroundColor As String = ColorTranslator.ToHtml(Color.Black)

    Public Sub UpdateWEConfigValues(config As String, display As String)
        SmoothingMode = CType(TryGetValue("smoothingMode", SmoothingMode.Default, config, display), SmoothingMode)
        CompositingQuality = CType(TryGetValue("compositingQuality", CompositingQuality.Default, config, display), CompositingQuality)
        InterpolationMode = CType(TryGetValue("interpolationMode", InterpolationMode.Default, config, display), InterpolationMode)
        PixelOffsetMode = CType(TryGetValue("pixelOffsetMode", PixelOffsetMode.Default, config, display), PixelOffsetMode)
        LEDShape = CType(TryGetValue("ledShape", LEDShape.Rectangle, config, display), LEDShape)
        RoundedRectangleCornerRadius = CInt(TryGetValue("roundedRectangleRadius", 0, config, display))
        LEDPadding = CSng(TryGetValue("ledPadding", 0F, config, display))
        TimerIntervals = CInt(TryGetValue("ledUpdateInterval", 30, config, display))
        IPAddress = CStr(TryGetValue("sdkIpAddress", "127.0.0.1", config, display))
        Port = CInt(TryGetValue("sdkPort", 6742, config, display))
        DeviceName = CStr(TryGetValue("deviceName", "Wallpaper1", config, display))
        BackgroundImage = CStr(TryGetValue("coverImage", Nothing, config, display))
        ImageFit = CType(TryGetValue("imageFit", ImageFit.None, config, display), ImageFit)
        BackgroundColor = ColorTranslator.ToHtml(CStr(TryGetValue("backgroundColor", "0 0 0", config, display)).ToColor)
        CpuUsagePauseValue = CInt(TryGetValue("cpuUsagePauseValue", 60, config, display))
    End Sub

    Public Function TryGetValue([property] As String, [default] As Object, Optional config As String = Nothing, Optional display As String = Nothing, Optional debug As Boolean = False) As Object
        Try
            Dim username As String = SystemInformation.UserName
            Dim mypath As String = Application.ExecutablePath.Replace("\", "/")
            Dim debugpath As String = "G:/Program Files (x86)/Steam/steamapps/common/wallpaper_engine/projects/myprojects/openrgbwallpaper/OpenRGBWallpaperEngine.exe"

            Dim json = JObject.Parse(File.ReadAllText(config))
            Dim item = json(username)("wproperties")(If(debug, debugpath, mypath))(display)([property])

            If item IsNot Nothing Then
                Return CType(item, Object)
            Else
                Return [default]
            End If

        Catch ex As Exception
            Logger.Log($"{ex.Message} {ex.StackTrace}")
            Return [default]
        End Try
    End Function

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

    <Extension>
    Public Function ToColor(customcolor As String) As Color
        Try
            Dim red = Math.Ceiling(CSng(customcolor.Split(" ")(0)) * 255)
            Dim green = Math.Ceiling(CSng(customcolor.Split(" ")(1)) * 255)
            Dim blue = Math.Ceiling(CSng(customcolor.Split(" ")(2)) * 255)
            Return Color.FromArgb(red, green, blue)
        Catch ex As Exception
            Return Color.Black
        End Try
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

    Public Function IsOpenRGBRunning() As Boolean
        Dim OpenRGB As Process = Process.GetProcessesByName("OpenRGB").FirstOrDefault
        Return Not OpenRGB Is Nothing
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