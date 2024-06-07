Imports System.ComponentModel
Imports System.IO
Imports Color = System.Drawing.Color

Public Class frmWallpaper

    Dim OffColor As Color = Color.FromArgb(255, 0, 0, 0)
    Dim configFile As String = WallpaperEngineConfig()
    Dim monitordetection As String = "devicepath"
    Dim display As String = ScreenDevicePath
    Dim configLastDate As Date = Now

    Public oRgbClient As OpenRgbClient = Nothing
    Public cpuUsage As New PerformanceCounter("Processor", "% Processor Time", "_Total")

    Dim connectString As String = Nothing
    Dim drawErrorStringOnScreen As Boolean = True

    Private Sub frmWallpaper_Load(sender As Object, e As EventArgs) Handles MyBase.Load
        If configFile <> "error" Then
            monitordetection = TryGetUserSettings("monitordetection", "devicepath", configFile)
            Select Case monitordetection
                Case "devicepath"
                    display = ScreenDevicePath
                Case "managed"
                    display = ScreenManaged
                Case "layout"
                    display = ScreenLayout
            End Select

            UpdateWEConfigValues(configFile, display)

            configLastDate = File.GetLastWriteTime(configFile)
            tmUpdate.Interval = TimerIntervals
            BackColor = ColorTranslator.FromHtml(BackgroundColor)
            pbDiffuser.Image = If(Utils.BackgroundImage = Nothing, Nothing, Image.FromFile(Utils.BackgroundImage))
            pbDiffuser.SizeMode = Utils.SizeMode

            Connect()
        End If
    End Sub

    Public Sub Connect()
        If IsOpenRGBRunning() Then
            Try
                If oRgbClient IsNot Nothing Then If oRgbClient.Connected Then oRgbClient.Dispose()
                oRgbClient = New OpenRgbClient(IPAddress, Port, "Wallpaper Engine", True, 1000, protocolVersionNumber:=4)
                connectString = Nothing
                tmCheckOpenRGB.Stop()
            Catch ex As Exception
                Logger.Log($"{ex.Message} {ex.StackTrace}")
                connectString &= $"{vbCrLf}[{Now.ToString("hh:mm:ss tt")}] Connection attempt failed, Local OpenRGB server unavailable."
                tmCheckOpenRGB.Start()
            End Try
        Else
            connectString &= $"{vbCrLf}[{Now.ToString("hh:mm:ss tt")}] Connection attempt failed, OpenRGB isn't running."
            tmCheckOpenRGB.Start()
        End If
    End Sub

    Private Function HighCpuUsage() As Boolean
        Return CInt(Math.Ceiling(cpuUsage.NextValue)) >= CpuUsagePauseValue
    End Function

    Private Sub tmUpdate_Tick(sender As Object, e As EventArgs) Handles tmUpdate.Tick
        If Not HighCpuUsage() Then Invalidate()
    End Sub

    Private Sub frmWallpaper_Closing(sender As Object, e As CancelEventArgs) Handles MyBase.Closing
        If oRgbClient IsNot Nothing Then
            If oRgbClient.Connected Then oRgbClient.Dispose()
        End If
    End Sub

    Private Sub PrepareGraphics(graphic As Graphics)
        graphic.SmoothingMode = SmoothingMode
        graphic.CompositingQuality = CompositingQuality
        graphic.InterpolationMode = InterpolationMode
        graphic.PixelOffsetMode = PixelOffsetMode
    End Sub

    Protected Overrides Sub OnPaint(e As PaintEventArgs)
        Dim graphic As Graphics = e.Graphics
        PrepareGraphics(graphic)
        graphic.Clear(BackColor)

        Try
            If oRgbClient IsNot Nothing Then
                If oRgbClient.Connected Then
                    Dim wallpaper = oRgbClient.GetAllControllerData.SingleOrDefault(Function(x) x.Name = DeviceName)
                    If wallpaper Is Nothing Then wallpaper = oRgbClient.GetAllControllerData.FirstOrDefault() 'Get the first device when couldn't find the target device
                    Dim oMatrix = wallpaper.Zones.FirstOrDefault.MatrixMap

                    Dim Width As Integer = oMatrix.Width
                    Dim Height As Integer = oMatrix.Height

                    Dim rectangleSize As New SizeF(ClientRectangle.Width / (wallpaper.Leds.Count / Height), ClientRectangle.Height / Height)

                    Dim matrix(Width - 1, Height - 1) As String
                    Dim count As Integer = 0
                    For j As Integer = 0 To matrix.GetUpperBound(0)
                        For i As Integer = 0 To matrix.GetUpperBound(0)
                            Dim rgbColor = wallpaper.Colors(count).ToColor
                            If rgbColor <> OffColor Then
                                Using sb As New SolidBrush(rgbColor)
                                    Dim X As Single = rectangleSize.Width * i
                                    Dim Y As Single = rectangleSize.Height * j
                                    Dim W As Single = rectangleSize.Width
                                    Dim H As Single = rectangleSize.Height
                                    Dim P As Single = LEDPadding

                                    Select Case Utils.LEDShape
                                        Case LEDShape.Rectangle
                                            graphic.FillRectangle(sb, New RectangleF(X + P, Y + P, W - P, H - P))
                                        Case LEDShape.RoundedRectangle
                                            graphic.FillRoundedRectangle(sb, New Rectangle(X + P, Y + P, W - P, H - P), RoundedRectangleCornerRadius)
                                        Case LEDShape.Sphere
                                            graphic.FillEllipse(sb, New RectangleF(X + P, Y + P, W - P, H - P))
                                    End Select
                                End Using
                            End If

                            count += 1
                            If count >= wallpaper.Leds.Count Then count = 0
                        Next
                    Next
                End If
            End If
        Catch ex As Exception
            Logger.Log($"{ex.Message} {ex.StackTrace}")
        End Try

        Try
            If drawErrorStringOnScreen AndAlso connectString <> Nothing Then
                TextRenderer.DrawText(graphic, connectString, Font, New Point(20, 1), Color.White)
            End If
        Catch ex As Exception
            Logger.Log($"{ex.Message} {ex.StackTrace}")
        End Try

        MyBase.OnPaint(e)
    End Sub

    Private Sub tmCheckOpenRGB_Tick(sender As Object, e As EventArgs) Handles tmCheckOpenRGB.Tick
        connectString &= $"{vbCrLf}[{Now.ToString("hh:mm:ss tt")}] Attempting to connect to local OpenRGB server."
        Connect()
    End Sub

    Private Sub tmConfig_Tick(sender As Object, e As EventArgs) Handles tmConfig.Tick
        Try
            Dim configDate As Date = File.GetLastWriteTime(configFile)
            If configLastDate <> configDate Then
                configLastDate = configDate
                UpdateWEConfigValues(configFile, display)

                configLastDate = File.GetLastWriteTime(configFile)
                tmUpdate.Interval = TimerIntervals
                BackColor = ColorTranslator.FromHtml(BackgroundColor)
                pbDiffuser.Image = If(Utils.BackgroundImage = Nothing, Nothing, Image.FromFile(Utils.BackgroundImage))
                pbDiffuser.SizeMode = Utils.SizeMode
            End If
        Catch ex As Exception
            Logger.Log($"{ex.Message} {ex.StackTrace}")
        End Try
    End Sub

End Class