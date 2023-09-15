from vapor import session, renderer, dataset, camera

ses = session.Session()

data = ses.OpenDataset(dataset.VDC, "/not/found/data.vdc")
print(f"Not found data = '{data}'")

# ses.Load("/Users/stasj/Work/sessions/time-empty.vs3")
# data = ses.GetDatasets()[0]
data = ses.OpenDataset(dataset.VDC, "/Users/stasj/Work/data/time/time.vdc")

ren = data.NewRenderer(renderer.TwoDDataRenderer)
# ren = ses.NewRenderer(renderer.TwoDDataRenderer, "time.vdc")
ren.SetEnabled(True)

ses.Render("out-python-cppyy.png")
ses.Show()

ren.SetRefinementLevel(3)
ren.SetCompressionLevel(3)
tf:renderer.TransferFunction = ren.GetPrimaryTransferFunction()
tf.LoadBuiltinColormap('Sequential/amp')
tf.SetOpacityList([1, 0, 1])
print(f"TF Mapping Range for {ren.GetVariableName()} is {tf.GetMinMapValue()} - {tf.GetMaxMapValue()}")

ses.SetTimestep(1)
ses.Render("out-python-cppyy-2.png")

ses.SetResolution(300,300)
ses.Save("out-session-time.vs3")
ses.Show()



#################################
# Another session

ses2 = session.Session()
maycontrol = ses2.OpenDataset(dataset.VDC, "/Users/stasj/Work/data/24Maycontrol.01/24Maycontrol.01.vdc")

vol = maycontrol.NewRenderer(renderer.VolumeRenderer)
print(f"Volume var name = '{vol.GetVariableName()}'")

cam = ses2.GetCamera()
cam.LookAt((-2.5, -2.5, 20), (-2.5,-2.5, 2.5))

ses2.Render("out-python-cppyy-maycontrol.png")
ses2.Save("out-session-dbz.vs3")

