using Microsoft.Owin;
using Owin;

[assembly: OwinStartupAttribute(typeof(IoTTemperatureDemo.Startup))]
namespace IoTTemperatureDemo
{
    public partial class Startup
    {
        public void Configuration(IAppBuilder app)
        {
        }
    }
}
