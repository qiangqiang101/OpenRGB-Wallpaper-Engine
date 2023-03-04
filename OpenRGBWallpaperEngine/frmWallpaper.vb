Imports System.ComponentModel
Imports System.IO
Imports Color = System.Drawing.Color

Public Class frmWallpaper

    Dim OffColor As Color = Color.FromArgb(255, 0, 0, 0)
    Dim configFile As String = WallpaperEngineConfig()
    Dim display As String = ScreenDevicePath
    Dim configLastDate As Date = Now

    Public oRgbClient As OpenRgbClient = Nothing
    Public BackImg As Image = Nothing
    Public ImgFit As ImageFit = ImageFit.Stretch
    Public cpuUsage As New PerformanceCounter("Processor", "% Processor Time", "_Total")

    Private Sub frmWallpaper_Load(sender As Object, e As EventArgs) Handles MyBase.Load
        If configFile <> "error" Then
            UpdateWEConfigValues(configFile, display)

            configLastDate = File.GetLastWriteTime(configFile)
            tmUpdate.Interval = TimerIntervals
            BackColor = ColorTranslator.FromHtml(BackgroundColor)
            BackImg = If(Utils.BackgroundImage = Nothing, Nothing, Image.FromFile(Utils.BackgroundImage))
            ImgFit = Utils.ImageFit
            If BackImg IsNot Nothing Then If ImgFit = ImageFit.Fit Then BackImg = BackImg.ResizeImage(ClientRectangle.Size, True)

            Connect()
        End If
    End Sub

    Public Sub Connect()
        If IsOpenRGBRunning() Then
            Try
                If oRgbClient IsNot Nothing Then If oRgbClient.Connected Then oRgbClient.Dispose()

                oRgbClient = New OpenRgbClient(IPAddress, Port, DeviceName, True, protocolVersion:=2)
            Catch ex As Exception
                Logger.Log($"{ex.Message} {ex.StackTrace}")
                tmCheckOpenRGB.Start()
            End Try
        Else
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
                    Dim wallpaper = oRgbClient.GetAllControllerData.Where(Function(x) x.Name = DeviceName).FirstOrDefault
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
            If BackImg IsNot Nothing Then
                Select Case ImgFit
                    Case ImageFit.None
                        graphic.DrawImage(BackImg, ClientRectangle.X, ClientRectangle.Y, New Rectangle(0, 0, ClientRectangle.Width, ClientRectangle.Height), GraphicsUnit.Pixel)
                    Case ImageFit.Center
                        Dim imgSize As Integer = BackImg.Width + BackImg.Height
                        Dim crSize As Integer = ClientRectangle.Width + ClientRectangle.Height
                        Dim iX As Integer = (ClientRectangle.Width - BackImg.Width) / 2
                        Dim iY As Integer = (ClientRectangle.Height - BackImg.Height) / 2

                        If crSize > imgSize Then
                            graphic.DrawImage(BackImg, iX, iY, New Rectangle(0, 0, ClientRectangle.Width, ClientRectangle.Height), GraphicsUnit.Pixel)
                        Else
                            graphic.DrawImage(BackImg, iX, iY, New Rectangle(0, 0, BackImg.Width, BackImg.Height), GraphicsUnit.Pixel)
                        End If
                    Case ImageFit.Stretch
                        graphic.DrawImage(BackImg, New RectangleF(ClientRectangle.X, ClientRectangle.Y, ClientRectangle.Width, ClientRectangle.Height), New RectangleF(0, 0, BackImg.Width, BackImg.Height), GraphicsUnit.Pixel)
                    Case ImageFit.Fill, ImageFit.Fit
                        Dim aspectRatio As Double
                        Dim newHeight, newWidth As Integer
                        Dim maxWidth As Integer = Width
                        Dim maxHeight As Integer = Width

                        If BackImg.Width > maxWidth Or BackImg.Height > maxHeight Then
                            If BackImg.Width >= BackImg.Height Then ' image is wider than tall
                                newWidth = maxWidth
                                aspectRatio = BackImg.Width / maxWidth
                                newHeight = CInt(BackImg.Height / aspectRatio)
                            Else ' image is taller than wide
                                newHeight = maxHeight
                                aspectRatio = BackImg.Height / maxHeight
                                newWidth = CInt(BackImg.Width / aspectRatio)
                            End If
                        Else
                            If BackImg.Width > BackImg.Height Then
                                newWidth = maxWidth
                                aspectRatio = BackImg.Width / maxWidth
                                newHeight = CInt(BackImg.Height / aspectRatio)
                            Else
                                newHeight = maxHeight
                                aspectRatio = BackImg.Height / maxHeight
                                newWidth = CInt(BackImg.Width / aspectRatio)
                            End If
                        End If

                        Dim newX As Integer = (Width - newWidth) / 2
                        Dim newY As Integer = (Height - newHeight) / 2

                        graphic.DrawImage(BackImg, New RectangleF(newX, newY, newWidth, newHeight))
                End Select

            End If
        Catch ex As Exception
            Logger.Log($"{ex.Message} {ex.StackTrace}")
        End Try

        MyBase.OnPaint(e)
    End Sub

    Private Sub tmCheckOpenRGB_Tick(sender As Object, e As EventArgs) Handles tmCheckOpenRGB.Tick
        Connect()
        tmCheckOpenRGB.Stop()
    End Sub

    Private Sub tmConfig_Tick(sender As Object, e As EventArgs) Handles tmConfig.Tick
        Dim configDate As Date = File.GetLastWriteTime(configFile)
        If configLastDate <> configDate Then
            configLastDate = configDate
            UpdateWEConfigValues(configFile, display)

            configLastDate = File.GetLastWriteTime(configFile)
            tmUpdate.Interval = TimerIntervals
            BackColor = ColorTranslator.FromHtml(BackgroundColor)
            BackImg = If(Utils.BackgroundImage = Nothing, Nothing, Image.FromFile(Utils.BackgroundImage))
            ImgFit = Utils.ImageFit
            If BackImg IsNot Nothing Then If ImgFit = ImageFit.Fit Then BackImg = BackImg.ResizeImage(ClientRectangle.Size, True)
        End If
    End Sub

End Class