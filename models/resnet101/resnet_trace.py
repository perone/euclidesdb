from torchvision.models import resnet
import torch.utils.model_zoo as model_zoo

import torch

import torch.nn.functional as F


class ResnetFeature(resnet.ResNet):
    def forward(self, x):
        x = self.conv1(x)
        x = self.bn1(x)
        x = self.relu(x)
        x = self.maxpool(x)

        x = self.layer1(x)
        x = self.layer2(x)
        x = self.layer3(x)
        x = self.layer4(x)

        x = self.avgpool(x)
        x_feat = x.view(x.size(0), -1)
        x = self.fc(x_feat)
        x = F.softmax(x, dim=1)

        return x, x_feat


def resnet101(pretrained=False, **kwargs):
    model = ResnetFeature(resnet.Bottleneck, [3, 4, 23, 3], **kwargs)
    if pretrained:
        model.load_state_dict(model_zoo.load_url(resnet.model_urls['resnet101']))
    return model


class Resnet101Module(torch.jit.ScriptModule):
    def __init__(self):
        super(Resnet101Module, self).__init__()
        self.means = torch.nn.Parameter(torch.tensor([0.485, 0.456, 0.406])
                                        .resize_(1, 3, 1, 1))
        self.stds = torch.nn.Parameter(torch.tensor([0.229, 0.224, 0.225])
                                        .resize_(1, 3, 1, 1))
        resnet_model = resnet101(pretrained=True)
        resnet_model.eval()
        self.resnet = torch.jit.trace(resnet_model,
                                      torch.rand(1, 3, 224, 224))

    @torch.jit.script_method
    def helper(self, input):
        return self.resnet((input - self.means) / self.stds)

    @torch.jit.script_method
    def forward(self, input):
        return self.helper(input)


model = Resnet101Module()
model.eval()
traced_net = torch.jit.trace(model,
                             torch.rand(1, 3, 224, 224))
traced_net.save("resnet101.pth")
