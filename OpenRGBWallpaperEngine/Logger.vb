Public NotInheritable Class Logger

    Public Shared Sub Log(message As String)
        IO.File.AppendAllText(IO.Path.Combine($".\", $"{Now.ToString("dd-MM-yyyy")}.log"), $"{Now.ToShortTimeString}: {message}{Environment.NewLine}")
    End Sub

End Class