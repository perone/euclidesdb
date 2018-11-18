from torchvision.models import vgg
import torch.utils.model_zoo as model_zoo

import torchvision
import torch
from torch import nn
import torch.nn.functional as F


class VGG16Feature(vgg.VGG):
    def __init__(self, features, num_classes=1000, init_weights=True):
        super(VGG16Feature, self).__init__(features, num_classes, init_weights)
        self.features = features
        self.classifier = nn.Sequential(
            nn.Linear(512 * 7 * 7, 4096),
            nn.ReLU(True),
            nn.Dropout(),
            nn.Linear(4096, 4096),
            nn.ReLU(True),
            nn.Dropout(),
        )

        self.classifier_final = nn.Sequential(
            nn.Linear(4096, num_classes),
            nn.Softmax(dim=0),
        )

        if init_weights:
            self._initialize_weights()

    def forward(self, x):
        x = self.features(x)
        x = x.view(x.size(0), -1)
        x_feats = self.classifier(x)
        x = self.classifier_final(x_feats)
        return x, x_feats


def vgg16(pretrained=False, **kwargs):
    if pretrained:
        kwargs['init_weights'] = False
    model = VGG16Feature(vgg.make_layers(vgg.cfg['D'], batch_norm=True), **kwargs)

    if pretrained:
        # Stitching to split the features from predictions
        state_dict = model_zoo.load_url(vgg.model_urls['vgg16_bn'])
        state_dict["classifier_final.0.weight"] = state_dict["classifier.6.weight"]
        state_dict["classifier_final.0.bias"] = state_dict["classifier.6.bias"]
        del state_dict["classifier.6.weight"]
        del state_dict["classifier.6.bias"]
        model.load_state_dict(state_dict)
    return model


class VGG16Module(torch.jit.ScriptModule):
    def __init__(self):
        super(VGG16Module, self).__init__()
        self.means = torch.nn.Parameter(torch.tensor([0.485, 0.456, 0.406])
                                        .resize_(1, 3, 1, 1))
        self.stds = torch.nn.Parameter(torch.tensor([0.229, 0.224, 0.225])
                                        .resize_(1, 3, 1, 1))
        vgg16_model = vgg16(pretrained=True)
        vgg16_model.eval()
        self.vgg16 = torch.jit.trace(vgg16_model,
                                      torch.rand(1, 3, 224, 224))

    @torch.jit.script_method
    def helper(self, input):
        return self.vgg16((input - self.means) / self.stds)

    @torch.jit.script_method
    def forward(self, input):
        return self.helper(input)


x = vgg16(pretrained=True)


model = VGG16Module()
model.eval()
traced_net = torch.jit.trace(model,
                             torch.rand(1, 3, 224, 224))
traced_net.save("vgg16.pth")
